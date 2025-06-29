#pragma once

//================================================================================================//
// FORWARD DECLARATIONS
//================================================================================================//

class Timer;

//================================================================================================//
// INCLUDES
//================================================================================================//

#include <cstdint>
#include <cstddef>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

namespace zct {

#define TIMER_LOG_LEVEL LOG_LEVEL_WRN

//================================================================================================//
// CLASS DECLARATION
//================================================================================================//

template <typename EventType>
class Timer {
public:

    /**
     * Create a new timer and associate it with an event.
     * 
     * The timer will not be running after creation.
     * 
     * \param event The event to fire when the timer expires. This is copied into the timer, so the lifetime of the
     *              event does not need to be longer than the timer.
     */
    Timer(const EventType& event) :
        m_event(event)
    {
    }

    /**
     * Start the timer in reoccurring mode. The timer will expire for the first time
     * after period_ms from when this is called, and then period_ms after that.
     * 
     * @param period_ms The period of the timer. Set to -1 for a one-shot timer, or 0/positive for a recurring timer.
     * @param eventRaw The event to fire when the timer expires. This is copied into the timer, so the lifetime of the
     *                 event does not need to be longer than the timer.
     * @param eventRawSize The size of the event in bytes, i.e. sizeof(MyEventType).
    */
    void start(int64_t period_ms) {
        // Convert ms to ticks
        start(period_ms, period_ms);
    }

    /**
     * Start the timer in either one-shot or reoccurring mode.
     * 
     * @param startDuration_ms The time to wait before the first expiry. Must either be 0 (no-wait) or positive.
     * @param period_ms The period of the timer. Set to -1 for a one-shot timer, or 0/positive for a recurring timer.
    */
    void start(int64_t startDuration_ms, int64_t period_ms) {
        LOG_MODULE_DECLARE(zct_Timer, TIMER_LOG_LEVEL);
        __ASSERT_NO_MSG(startDuration_ms >= 0); // Start time can be 0, which means the timer will fire immediately. Can't be negative!
        __ASSERT_NO_MSG(period_ms >= -1); // Period can be -1, which means the timer will not repeat
        
        if (!this->m_isRegistered) {
            LOG_WRN("Timer %p is not registered with a timer manager. Expiry events will not be handled.", this);
        }

        this->startTime_ticks = k_uptime_ticks();
        // Use ceil and not floor to guarantee a minimum delay
        this->nextExpiryTime_ticks = this->startTime_ticks + k_ms_to_ticks_ceil64(startDuration_ms);
        if (period_ms == -1) {
            this->period_ticks = -1;
        } else {
            this->period_ticks = k_ms_to_ticks_ceil64(period_ms);
        }
        this->m_isRunning = true;
    }

    /**
     * Stop the timer. This will prevent the timer from expiring until
     * start() is called again.
     * 
     * This does not deregister the timer from the timer manager (if registered), nor
     * does this clear the event that is saved in the timer for when it expires.
     */
    void stop() {
        this->m_isRunning = false;
        this->period_ticks = -1;
        this->startTime_ticks = 0;
        this->nextExpiryTime_ticks = 0;
    }

    /**
     * Check if the timer is running.
     * 
     * @return true if the timer is running, false otherwise.
    */
    bool isRunning() const { return this->m_isRunning; }

    /**
     * Designed to be called by the state machine when the timer expires.
     * 
     * This will either:
     * 1. Stop the timer if it is a one-shot timer.
     * 2. Update the next expiry time if it is a recurring timer.
    */
    void updateAfterExpiry() {
        LOG_MODULE_DECLARE(zct_Timer, TIMER_LOG_LEVEL);
        if (this->period_ticks == -1)
        {
            // Timer was one-shot, so stop it
            this->m_isRunning = false;
        }
        else
        {
            // Update expiry time based on the period
            LOG_DBG("Updating timer expiry time. Period: %lld. Next expiry time before update: %lld.", this->period_ticks, this->nextExpiryTime_ticks);
            this->nextExpiryTime_ticks += this->period_ticks;
            LOG_DBG("Next expiry time after update: %lld.", this->nextExpiryTime_ticks);
        }
    }

    /**
     * Gets a pointer to the event which is saved in the timer.
     * Needs to be cast by the user back to the correct type.
     */
    const EventType& getEvent() const { return this->m_event; }

    /**
     * Get the next expiry time of the timer.
     * 
     * @return The next expiry time of the timer in ticks.
     */
    int64_t getNextExpiryTimeTicks() const { return this->nextExpiryTime_ticks; }

    /**
     * Set the isRegistered flag. Used to log warnings if a timer is
     * started when it is not registered with any timer managers.
     * 
     * @param isRegistered The value to set the isRegistered flag to.
     */
    void setIsRegistered(bool isRegistered) { this->m_isRegistered = isRegistered; }

    /**
     * Get the isRegistered flag.
     * 
     * @return The value of the isRegistered flag.
     */
    bool getIsRegistered() const { return this->m_isRegistered; }

protected:
    int64_t period_ticks = 0;
    int64_t startTime_ticks = 0;
    int64_t nextExpiryTime_ticks = 0;
    bool m_isRunning = false;
    EventType m_event;
    bool m_isRegistered = false;
};

} // namespace zct