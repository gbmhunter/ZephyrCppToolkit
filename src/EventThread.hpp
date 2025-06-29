#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "Timer.hpp"
#include "TimerManager.hpp"

namespace zephyr_cpp_toolkit {

template <typename EventType>
class EventThread {

public:
    /**
     * Create a new event thread.EventThread
     * @param threadStack The stack to use for the thread.
     * @param threadStackSize The size of the stack to use for the thread.
     * @param eventQueueBuffer The event queue to use for the thread.
     * @param eventQueueBufferNumItems The number of items in the event queue.
     */
    EventThread(k_thread_stack_t* threadStack, size_t threadStackSize, EventType* eventQueueBuffer, size_t eventQueueBufferNumItems) :
        m_timerManager(10)
    {
        LOG_MODULE_DECLARE(EventThread);
        // Create message queue

        // void* msgqStorage = k_malloc(sizeof(EventType)*eventQueueNumItems);
        // __ASSERT_NO_MSG(msgqStorage != nullptr);
        k_msgq_init(&m_threadMsgQueue, (char*)eventQueueBuffer, sizeof(EventType), eventQueueBufferNumItems);

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
        // Name the thread for easier debugging
        k_thread_name_set(&m_thread, "ThreadedSM");
        LOG_DBG("Threaded state machine initialized.");
    };

    /**
     * Call to block and wait for an event. An event can either be:
     * - A internal timer timeout event.
     * - An external event (sent from another thread).
     */
    EventType waitForEvent() {
        LOG_MODULE_DECLARE(EventThread);

        LOG_DBG("Threaded state machine thread started.");

        // Get the next timer to expire
        auto nextTimerInfo = m_timerManager.getNextExpiringTimer();

        // Handle all expired timers
        if (nextTimerInfo.m_timer != nullptr && nextTimerInfo.m_durationToWaitUs == 0) {
            nextTimerInfo = m_timerManager.getNextExpiringTimer();
            LOG_DBG("Timer expired. Timer: %p.", nextTimerInfo.m_timer);
            // Get the timer event
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
            // LOG_DBG("Received event from queue. Event ID: %u.", static_cast<uint32_t>(obj->m_lastReceivedMsg.id));
            // We got a message from the queue, so we can handle it
            return rxEvent;
        } else if (queueRc == -EAGAIN) {
            // Queue timed out, which means we need to handle the timer expiry
            LOG_DBG("Queue timed out, which means we need to handle the timer expiry.");
            return nextTimerInfo.m_timer->getEvent();
        } else if (queueRc == -ENOMSG) {
            // This means the queue must have been purged
            __ASSERT(false, "Got -ENOMSG from queue, was not expecting this.");
        } else {
            __ASSERT(false, "Got unexpected return code from queue: %d.", queueRc);
        }
    }

private:

    /** The function needed by pass to Zephyr's thread API */
    static void threadFunction(void* arg1, void* arg2, void* arg3)
    {
        LOG_MODULE_DECLARE(EventThread);

        // First passed in argument is the instance of the class
        EventThread* obj = static_cast<EventThread*>(arg1);
        // Call the derived class's threadMain function
        obj->threadMain();
    }


    /** To be implemented by the derived class */
    virtual void threadMain() = 0;

    

    struct k_thread m_thread;
    static constexpr size_t STACK_SIZE = 1024;
    K_KERNEL_STACK_MEMBER(m_threadStack, STACK_SIZE);
    static constexpr int THREAD_PRIORITY = 7;

    struct k_msgq m_threadMsgQueue;


    TimerManager<EventType> m_timerManager;
};

} // namespace zephyr_cpp_toolkit
