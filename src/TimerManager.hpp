#pragma once

//================================================================================================//
// FORWARD DECLARATIONS
//================================================================================================//


//================================================================================================//
// INCLUDES
//================================================================================================//

#include <stdint.h>
#include <zephyr/kernel.h>

#include "Timer.hpp"

//================================================================================================//
// CLASS DECLARATION
//================================================================================================//

namespace zephyr_cpp_toolkit {

template <typename EventType>
class TimerManager {
public:

    /**
     * Create a new timer manager.
     * @param maxNumTimers The maximum number of timers that can be registered with the timer manager. Space for
     *                     this many pointers to timers will be allocated on the heap.
     */
    TimerManager(uint32_t maxNumTimers) {
        m_timers = new Timer<EventType>*[maxNumTimers];
        for (uint32_t i = 0; i < maxNumTimers; i++) {
            m_timers[i] = nullptr;
        }
        m_maxNumTimers = maxNumTimers;
    }

    /** Used as a return type for getNextExpiringTimer(). */
    struct TimerExpiryInfo {
        const Timer<EventType>* m_timer = nullptr;
        uint64_t m_durationToWaitUs = 0;

        // Explicit constructor
        TimerExpiryInfo(const Timer<EventType>* timer, uint64_t durationToWaitUs) : m_timer(timer), m_durationToWaitUs(durationToWaitUs) {}
    };


    /**
     * Registers a timer with the timer manager. The provided timer needs to exist for the duration of the registration.
     * @param timer The timer to register.
     */
    void registerTimer(Timer<EventType>& timer) {
        __ASSERT(m_numTimers < m_maxNumTimers, "Max number of timers of %u reached.", m_maxNumTimers);
        m_timers[m_numTimers] = &timer;
        m_numTimers++;
    }

    /**
     * Iterates over all registered timers and returns the expiry time of the timer that expires next.
     * If no timer is found, the 'timer' member of the returned struct will be nullptr. timer is guaranteed to
     * not be nullptr if durationToWaitUs is not UINT64_MAX.
     * @return A struct containing the timer that expires next and the duration to wait until that timer expires.
     */
    TimerExpiryInfo getNextExpiringTimer() {
        LOG_MODULE_DECLARE(TimerManager);

        // Set output to null in case no timer expired
        Timer<EventType>* expiredTimer = nullptr;
        uint64_t durationToWaitUs = 0;
        // Iterate through all registered timers, and find the one that is expiring next (if any)
        for(uint32_t i = 0; i < this->m_numTimers; i++) {
            Timer<EventType> * timer = this->m_timers[i];
            if (timer->isRunning()) {
                if (expiredTimer == nullptr || timer->getNextExpiryTimeTicks() < expiredTimer->getNextExpiryTimeTicks()) {
                    // LOG_DBG("Setting expired timer to %p.", timer);
                    expiredTimer = timer;
                }
            }
        }
        LOG_DBG("Expired timer: %p.", expiredTimer);

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
                expiredTimer->updateAfterExpiry();
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
    Timer<EventType> ** m_timers;
    uint32_t m_numTimers = 0;
    uint32_t m_maxNumTimers;
};

} // namespace zephyr_cpp_toolkit
