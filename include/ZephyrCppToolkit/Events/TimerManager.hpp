#pragma once

//================================================================================================//
// FORWARD DECLARATIONS
//================================================================================================//


//================================================================================================//
// INCLUDES
//================================================================================================//

// System includes
#include <stdint.h>

// 3rd party includes
#include <zephyr/kernel.h>

// Local includes
#include "Timer.hpp"

//================================================================================================//
// MACROS
//================================================================================================//

// Don't use constexpr here, it seg faults!
// static constexpr int LOG_LEVEL = LOG_LEVEL_DBG;
#define ZCT_TIMER_MANAGER_LOG_LEVEL LOG_LEVEL_WRN

//================================================================================================//
// CLASS DECLARATION
//================================================================================================//

namespace zct {

class TimerManager {
public:

    /**
     * Create a new timer manager.
     * 
     * Dynamically allocates memory for maxNumTimers pointers to timers.
     * 
     * @param maxNumTimers The maximum number of timers that can be registered with the timer manager. Space for
     *                     this many pointers to timers will be allocated on the heap.
     */
    TimerManager(uint32_t maxNumTimers) {
        LOG_MODULE_DECLARE(TimerManager, ZCT_TIMER_MANAGER_LOG_LEVEL);
        LOG_DBG("TimerManager constructor called.");
        m_timers = new Timer*[maxNumTimers];
        for (uint32_t i = 0; i < maxNumTimers; i++) {
            m_timers[i] = nullptr;
        }
        m_maxNumTimers = maxNumTimers;
        LOG_DBG("TimerManager constructor finished.");
    }

    ~TimerManager() {
        // Free the memory allocated in constructor.
        delete[] m_timers;
    }

    /** Used as a return type for getNextExpiringTimer(). */
    struct TimerExpiryInfo {
        Timer* m_timer = nullptr;
        uint64_t m_durationToWaitUs = 0;

        // Explicit constructor
        TimerExpiryInfo(Timer* timer, uint64_t durationToWaitUs) : m_timer(timer), m_durationToWaitUs(durationToWaitUs) {}
    };


    /**
     * Registers a timer with the timer manager. The provided timer needs to exist for the duration of the registration.
     * @param timer The timer to register.
     */
    void registerTimer(Timer& timer) {
        __ASSERT(m_numTimers < m_maxNumTimers, "Max number of timers of %u reached.", m_maxNumTimers);
        m_timers[m_numTimers] = &timer;
        m_numTimers++;
        // Make sure to set the isRegistered flag to true. This will prevent log warnings
        // if the timer is started when it is not registered with any timer managers.
        timer.setIsRegistered(true);
    }

    /**
     * Iterates over all registered timers and returns the expiry time of the timer that expires next.
     * If no timer is found, the 'timer' member of the returned struct will be nullptr. timer is guaranteed to
     * not be nullptr if durationToWaitUs is not UINT64_MAX.
     * 
     * This function does not update the timer's next expiry time if it finds an expired timer. It is up to the caller to do this when it decides that the timer expiry has been "handled".
     * 
     * \return A struct containing the timer that expires next and the duration to wait until that timer expires.
     */
    TimerExpiryInfo getNextExpiringTimer() {
        LOG_MODULE_DECLARE(TimerManager, ZCT_TIMER_MANAGER_LOG_LEVEL);
        LOG_DBG("getNextExpiringTimer() called. this: %p, m_numTimers: %u.", this, m_numTimers);

        // Set output to null in case no timer expired
        Timer* expiredTimer = nullptr;
        uint64_t durationToWaitUs = 0;
        // Iterate through all registered timers, and find the one that is expiring next (if any)
        for(uint32_t i = 0; i < this->m_numTimers; i++) {
            Timer* timer = this->m_timers[i];
            if (timer->isRunning()) {
                if (expiredTimer == nullptr || timer->getNextExpiryTimeTicks() < expiredTimer->getNextExpiryTimeTicks()) {
                    // LOG_DBG("Setting expired timer to %p.", timer);
                    expiredTimer = timer;
                }
            }
        }
        LOG_DBG("Expired timer: %p.\n", expiredTimer);

        // Convert the expiry time to a duration from now
        // Calculate time to wait for next timeout event
        durationToWaitUs = 0;

        // Ticks is the fundemental resolution that the kernel does operations at
        int64_t uptime_ticks = k_uptime_ticks();
        if (expiredTimer != nullptr) {
            if (expiredTimer->getNextExpiryTimeTicks() <= uptime_ticks) {
                durationToWaitUs = 0;
                LOG_DBG("Timer expired.");
                // Need to update the timer now that we have detected it has expired.
                // This will either stop the timer if it is a one-shot, or update the next expiry time
                // expiredTimer->updateAfterExpiry();
            } else {
                durationToWaitUs = k_ticks_to_us_ceil64(expiredTimer->getNextExpiryTimeTicks() - uptime_ticks);
                LOG_DBG("Time to wait in us: %llu.", durationToWaitUs);
            }
        }
        else
        {
            LOG_DBG("No timers running.");
        }
        // Save outputs
        return TimerManager::TimerExpiryInfo{expiredTimer, durationToWaitUs};
    }


protected:
    Timer** m_timers;
    uint32_t m_numTimers = 0;
    uint32_t m_maxNumTimers;
};

} // namespace zct
