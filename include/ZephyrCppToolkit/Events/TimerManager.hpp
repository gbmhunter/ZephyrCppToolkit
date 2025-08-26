#pragma once

//================================================================================================//
// INCLUDES
//================================================================================================//

// System includes
#include <stdint.h>

// 3rd party includes
#include <zephyr/kernel.h>

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

// Forward declarations
class Timer;

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
    TimerManager(uint32_t maxNumTimers);

    ~TimerManager();

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
    void registerTimer(Timer& timer);

    /**
     * Iterates over all registered timers and returns the expiry time of the timer that expires next.
     * If no timer is found, the 'timer' member of the returned struct will be nullptr. timer is guaranteed to
     * not be nullptr if durationToWaitUs is not UINT64_MAX.
     * 
     * This function does not update the timer's next expiry time if it finds an expired timer. It is up to the caller to do this when it decides that the timer expiry has been "handled".
     * 
     * \return A struct containing the timer that expires next and the duration to wait until that timer expires.
     */
    TimerExpiryInfo getNextExpiringTimer();


protected:
    Timer** m_timers;
    uint32_t m_numTimers = 0;
    uint32_t m_maxNumTimers;
};

} // namespace zct
