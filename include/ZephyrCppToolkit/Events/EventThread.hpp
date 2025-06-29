#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "Timer.hpp"
#include "TimerManager.hpp"

namespace zct {

template <typename EventType>
class EventThread {

public:

    static constexpr int LOG_LEVEL = LOG_LEVEL_DBG;

    /**
     * Create a new event thread.
     * 
     * Dynamically allocates memory for the event queue buffer.
     *
     * @param name The name of the thread. Used for logging purposes.
     * @param threadStack The stack to use for the thread.
     * @param threadStackSize The size of the stack provided.
     * @param eventQueueBufferNumItems The number of items in the event queue.
     */
    EventThread(const char* name, k_thread_stack_t* threadStack, size_t threadStackSize, size_t eventQueueBufferNumItems) :
        m_timerManager(10)
    {
        LOG_MODULE_DECLARE(zct_EventThread);
        m_name = name;
        // Create event queue buffer and then init queue with it
        m_eventQueueBuffer = new EventType[eventQueueBufferNumItems];
        __ASSERT_NO_MSG(m_eventQueueBuffer != nullptr);
        k_msgq_init(&m_threadMsgQueue, (char*)m_eventQueueBuffer, sizeof(EventType), eventQueueBufferNumItems);

        LOG_DBG("Initializing threaded state machine...");
        // Start the thread
        k_thread_create(
            &m_thread,
            threadStack,
            threadStackSize,
            threadFunction,
            this, // Pass in the instance of the class
            NULL,
            NULL,
            THREAD_PRIORITY,
            0,
            K_NO_WAIT);
        // Name the thread for easier debugging/logging
        k_thread_name_set(&m_thread, m_name);
        LOG_DBG("Threaded state machine initialized.");
    };

    ~EventThread() {
        LOG_MODULE_DECLARE(zct_EventThread);
        LOG_DBG("%s() called.", __FUNCTION__);
        k_thread_join(&m_thread, K_FOREVER);
    }

    /**
     * Call to block and wait for an event. An event can either be:
     * - A internal timer timeout event.
     * - An external event (sent from another thread).
     */
    EventType waitForEvent() {
        LOG_MODULE_DECLARE(zct_EventThread);
        LOG_DBG("%s() called.", __FUNCTION__);

        // Get the next timer to expire
        auto nextTimerInfo = m_timerManager.getNextExpiringTimer();

        // See if there is an already expired timer, is so, there is no need to block
        // on the event queue.
        if (nextTimerInfo.m_timer != nullptr && nextTimerInfo.m_durationToWaitUs == 0) {
            nextTimerInfo = m_timerManager.getNextExpiringTimer();
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
     * Send an event to this event thread.
     * 
     * @param event The event to send. It is copied into the event queue so it's lifetime only needs to be as long as this function call.
     */
    void sendEvent(const EventType& event) {
        k_msgq_put(&m_threadMsgQueue, &event, K_NO_WAIT);
    }

protected:

    /** The function needed by pass to Zephyr's thread API */
    static void threadFunction(void* arg1, void* arg2, void* arg3)
    {
        LOG_MODULE_DECLARE(zct_EventThread);

        // First passed in argument is the instance of the class
        EventThread* obj = static_cast<EventThread*>(arg1);
        // Call the derived class's threadMain function
        obj->threadMain();

        // If we get here, the user decided to exit the thread
    }


    /** To be implemented by the derived class */
    virtual void threadMain() = 0;

    const char* m_name = nullptr;
    void* m_eventQueueBuffer = nullptr;

    struct k_thread m_thread;
    static constexpr size_t STACK_SIZE = 1024;
    K_KERNEL_STACK_MEMBER(m_threadStack, STACK_SIZE);
    static constexpr int THREAD_PRIORITY = 7;

    struct k_msgq m_threadMsgQueue;


    TimerManager<EventType> m_timerManager;
};

} // namespace zct
