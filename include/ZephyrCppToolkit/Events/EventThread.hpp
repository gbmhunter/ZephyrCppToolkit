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
     * @param threadFunction The function to call when the thread is started. This should
     *                        do whatever setup you need and then call waitForEvent().
     * @param threadPriority The priority to assign to the thread.
     * @param eventQueueBufferNumItems The number of items in the event queue.
     */
    EventThread(
        const char* name,
        k_thread_stack_t* threadStack,
        size_t threadStackSize,
        std::function<void()> threadFunction,
        int threadPriority,
        size_t eventQueueBufferNumItems
    ) :
        m_threadFunction(threadFunction),
        m_timerManager(10)
    {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("EventThread constructor called.");
        m_name = name;

        // Create event queue buffer and then init queue with it
        m_eventQueueBuffer = new EventType[eventQueueBufferNumItems];
        __ASSERT_NO_MSG(m_eventQueueBuffer != nullptr);
        k_msgq_init(&m_threadMsgQueue, (char*)m_eventQueueBuffer, sizeof(EventType), eventQueueBufferNumItems);

        // Start the thread
        k_thread_create(
            &m_thread,
            threadStack,
            threadStackSize,
            staticThreadFunction,
            this, // Pass in the instance of the class
            NULL,
            NULL,
            threadPriority,
            0,
            K_NO_WAIT);
        // Name the thread for easier debugging/logging
        k_thread_name_set(&m_thread, m_name);
    };

    /**
     * Destroy the event thread. This will block until the thread has exited.
     * 
     * If you want this function to return, make sure you make the thread function return.
     * See \ref EventThread for more details.
     */
    ~EventThread() {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("%s() called.", __FUNCTION__);
        k_thread_join(&m_thread, K_FOREVER);
    }

    /**
     * Call to block and wait for an event. An event can either be:
     * - A internal timer timeout event.
     * - An external event (sent from another thread).
     * 
     * This should be called from the thread function that is passed in to the constructor,
     * once any initialisation you want done (e.g. setup some timers) has been done.
     * 
     * \return The event that was received. This should be handled in your thread function
     *      and then loop back to `waitForEvent()` to wait for the next event.
     */
    EventType waitForEvent() {
        LOG_MODULE_DECLARE(EventThread, ZCT_EVENT_THREAD_LOG_LEVEL);
        LOG_DBG("%s() called.", __FUNCTION__);

        // Get the next timer to expire
        auto nextTimerInfo = m_timerManager.getNextExpiringTimer();

        // See if there is an already expired timer, is so, there is no need to block
        // on the event queue.
        if (nextTimerInfo.m_timer != nullptr && nextTimerInfo.m_durationToWaitUs == 0) {
            LOG_DBG("Timer expired. Timer: %p.", nextTimerInfo.m_timer);
            // Get the timer event
            nextTimerInfo.m_timer->updateAfterExpiry();
            return nextTimerInfo.m_timer->getEvent();
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
            // We got a message from the queue, so we can handle it
            return rxEvent;
        } else if (queueRc == -EAGAIN) {
            // Queue timed out, which means we need to handle the timer expiry
            LOG_DBG("Queue timed out, which means we need to handle the timer expiry.");
            nextTimerInfo.m_timer->updateAfterExpiry();
            return nextTimerInfo.m_timer->getEvent();
        } else if (queueRc == -ENOMSG) {
            // This means the queue must have been purged
            __ASSERT(false, "Got -ENOMSG from queue, was not expecting this.");
        } else {
            __ASSERT(false, "Got unexpected return code from queue: %d.", queueRc);
        }
        __ASSERT(false, "Should not get here.");
        return EventType();
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
    TimerManager<EventType>& timerManager() {
        return m_timerManager;
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
        // Call the derived class's threadMain function
        obj->m_threadFunction();

        // If we get here, the user decided to exit the thread
    }


    const char* m_name = nullptr;
    void* m_eventQueueBuffer = nullptr;

    struct k_thread m_thread;

    struct k_msgq m_threadMsgQueue;

    std::function<void()> m_threadFunction;
    TimerManager<EventType> m_timerManager;
};

/**
 * \example ../examples/IntegrationTest/src/App.cpp
 * This is an example of how to use the \ref zct::EventThread class to create a thread that can wait for events.
 */

} // namespace zct
