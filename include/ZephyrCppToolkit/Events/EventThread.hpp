#pragma once

#include <functional>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "Timer.hpp"
#include "TimerManager.hpp"

namespace zct {

// Don't use constexpr here, it seg faults!
// static constexpr int LOG_LEVEL = LOG_LEVEL_DBG;
#define ZCT_EVENT_THREAD_LOG_LEVEL LOG_LEVEL_WRN

/**
 * \brief Use this class in your objects to create a thread that can wait for events.
 * 
 * This class spawns a new Zephyr thread.
 * 
 * The user is responsible for making sure the thread function returns if you want to destroy this
 * object. This is because the destructor blocks until the thread is complete. The best way
 * to do this is to implement a exit event and send it to the event thread. The event thread
 * returns when it receives the exit event.
 * 
 * Making sure to return from the thread function is only important if you are destroying the object, e.g. in testing.
 * 
 * Below is an example of how to use this class.
 * 
 * \include EventThreadExample/main.cpp
 */
template <typename EventType>
class EventThread {

public:

    /**
     * Create a new event thread.
     * 
     * Dynamically allocates memory for the event queue buffer. At the moment, you have to allocate
     * the thread stack yourself and pass it in. Once Zephyr's dynamic thread support is out of
     * experimental (and working), hopefully we can just pass in the desired stack size.
     *
     * @param name The name of the this event thread. Used for logging purposes.
     *      The Zephyr thread name will also be set to this name.
     * @param threadStack The stack to use for the thread. You can use `K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);` to declare a stack member in the parent class that encloses this event thread.
     * @param threadStackSize The size of the stack provided.
     * @param threadPriority The priority to assign to the thread.
     * @param eventQueueBufferNumItems The number of items in the event queue.
     */
    EventThread(
        const char* name,
        k_thread_stack_t* threadStack,
        size_t threadStackSize,
        int threadPriority,
        size_t eventQueueBufferNumItems
    ) :
        m_threadStack(threadStack),
        m_threadStackSize(threadStackSize),
        m_threadPriority(threadPriority),
        m_timerManager(10)
    {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("EventThread constructor called.");
        m_name = name;

        // Create event queue buffer and then init queue with it
        m_eventQueueBuffer = new EventType[eventQueueBufferNumItems];
        __ASSERT_NO_MSG(m_eventQueueBuffer != nullptr);
        k_msgq_init(&m_threadMsgQueue, (char*)m_eventQueueBuffer, sizeof(EventType), eventQueueBufferNumItems);
    };

    /**
     * Start the event thread. This creates and starts the Zephyr thread which will begin
     * running the event loop.
     * 
     * This should be called after the constructor and after any setup (like registering timers)
     * has been completed.
     */
    void start() {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("EventThread start() called.");
        
        // Start the thread
        k_thread_create(
            &m_thread,
            m_threadStack,
            m_threadStackSize,
            staticThreadFunction,
            this, // Pass in the instance of the class
            NULL,
            NULL,
            m_threadPriority,
            0,
            K_NO_WAIT);
        // Name the thread for easier debugging/logging
        k_thread_name_set(&m_thread, m_name);
    }

    /**
     * Destroy the event thread. This will block until the thread has exited.
     * 
     * If you want this function to return, make sure the event loop exits.
     * The best way to do this is to call exitEventLoop() from a timer callback
     * or external event handler.
     */
    ~EventThread() {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("%s() called.", __FUNCTION__);
        k_thread_join(&m_thread, K_FOREVER);
    }

    /**
     * Set the callback function to be called when external events are received.
     * 
     * \param callback The callback function to call when an external event is received.
     *                 The callback will be passed the received event.
     */
    void onExternalEvent(std::function<void(const EventType&)> callback) {
        m_externalEventCallback = callback;
    }

    /**
     * Start the event loop. This function never returns and should be called from the thread function that is passed in to the constructor.
     * 
     * This function will:
     * - Handle expired timers by calling their callbacks
     * - Handle external events by calling the registered external event callback (call onExternalEvent() to register a callback).
     * 
     * This should be called from the thread function that is passed in to the constructor,
     * once any initialisation you want done (e.g. setup some timers) has been done.
     */
    void runEventLoop() {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("%s() called.", __FUNCTION__);

        while (!m_exitEventLoop) {
            // Get the next timer to expire
            auto nextTimerInfo = m_timerManager.getNextExpiringTimer();

        // Check for expired timers and call their callbacks
        while (nextTimerInfo.m_timer != nullptr && nextTimerInfo.m_durationToWaitUs == 0) {
            LOG_DBG("Timer expired. Timer: %p.", nextTimerInfo.m_timer);
            // Update the timer after expiry
            nextTimerInfo.m_timer->updateAfterExpiry();
            
            // Call the callback
            const auto& callback = nextTimerInfo.m_timer->getExpiryCallback();
            if (callback) {
                callback();
                // If the callback calls exitEventLoop(), we will exit the event loop
                // and return from runEventLoop().
                if (m_exitEventLoop) {
                    return;
                }
            }
            
            // Check for the next expired timer
            nextTimerInfo = m_timerManager.getNextExpiringTimer();
        }

        // If we get here, we have handled all expired timers.
        k_timeout_t timeout;
        if (nextTimerInfo.m_timer != nullptr) {
            timeout = Z_TIMEOUT_US(nextTimerInfo.m_durationToWaitUs);
        } else {
            timeout = K_FOREVER;
        }

        // Block on message queue until next timer expiry
        // Create storage in the case we receive an external event
        EventType rxEvent;
        int queueRc = k_msgq_get(&m_threadMsgQueue, &rxEvent, timeout);
        if (queueRc == 0) {
            // We got a message from the queue, call the external event callback
            if (m_externalEventCallback) {
                m_externalEventCallback(rxEvent);
                // If the callback calls exitEventLoop(), we will exit the event loop
                // and return from runEventLoop().
                if (m_exitEventLoop) {
                    return;
                }
            } else {
                LOG_WRN("Received external event in event thread \"%s\" but no external event callback is registered.", m_name);
            }
            continue;
        } else if (queueRc == -EAGAIN) {
            // Queue timed out, which means we need to handle the timer expiry,
            // jump back to start of while loop to handle the timer expiry
            LOG_DBG("Queue timed out, which means we need to handle timer expiry.");
            continue;
        } else if (queueRc == -ENOMSG) {
            // This means the queue must have been purged
            __ASSERT(false, "Got -ENOMSG from queue, was not expecting this.");
        } else {
            __ASSERT(false, "Got unexpected return code from queue: %d.", queueRc);
        }
        } // End of while (true) loop
    }

    /**
     * Send an event to this event thread. This can be called from any other thread to send an
     * event to this event thread.
     * 
     * \note This function is thread safe.
     * \param event The event to send. It is copied into the event queue so it's lifetime only needs to be as long as this function call.
     */
    void sendEvent(const EventType& event) {
        k_msgq_put(&m_threadMsgQueue, &event, K_NO_WAIT);
    }

    /**
     * Call to get the timer manager for this event thread. This is useful when you want
     * to create timers and then register them with the event thread.
     * 
     * Typically you would call:
     * 
     * \code
     * myEventThread.timerManager().registerTimer(myTimer);
     * \endcode
     * 
     * \return A reference to the timer manager for this event thread.
     */
    TimerManager& timerManager() {
        return m_timerManager;
    }

    /**
     * Exit the event loop. This will cause runEventLoop() to return.
     * This function must be called from the event thread context (i.e. from a timer
     * callback or from an external event handler).
     * 
     * This will make the event loop return from runEventLoop() as soon as the timer callback
     * or external event handler returns.
     * 
     * Use this in tests to cleanly exit from the event loop and it's corresponding thread.
     */
    void exitEventLoop() {
        m_exitEventLoop = true;
    }

protected:

    /** The function needed by pass to Zephyr's thread API */
    static void staticThreadFunction(void* arg1, void* arg2, void* arg3)
    {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

        // First passed in argument is the instance of the class
        EventThread* obj = static_cast<EventThread*>(arg1);
        // Run the event loop. This should not return unless the user calls exitEventLoop().
        obj->runEventLoop();
        // If we get here, the user decided to exit the thread
    }

    const char* m_name = nullptr;
    void* m_eventQueueBuffer = nullptr;

    struct k_thread m_thread;
    struct k_msgq m_threadMsgQueue;

    k_thread_stack_t* m_threadStack;
    size_t m_threadStackSize;
    int m_threadPriority;

    TimerManager m_timerManager;
    std::function<void(const EventType&)> m_externalEventCallback = nullptr;

    /**
     * Used to signal from exitEventLoop() to the code in the runEventLoop() function to exit.
     */
    bool m_exitEventLoop = false;
};

/**
 * \example ../examples/IntegrationTest/src/App.cpp
 * This is an example of how to use the \ref zct::EventThread class to create a thread that can wait for events.
 */

} // namespace zct
