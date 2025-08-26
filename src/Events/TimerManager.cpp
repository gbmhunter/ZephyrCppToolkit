//================================================================================================//
// FORWARD DECLARATIONS
//================================================================================================//


//================================================================================================//
// INCLUDES
//================================================================================================//

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "ZephyrCppToolkit/Events/TimerManager.hpp"
#include "ZephyrCppToolkit/Events/Timer.hpp"

namespace zct {

LOG_MODULE_REGISTER(TimerManager, LOG_LEVEL_DBG);

TimerManager::TimerManager(uint32_t maxNumTimers) {
    LOG_MODULE_DECLARE(TimerManager, ZCT_TIMER_MANAGER_LOG_LEVEL);
    LOG_DBG("TimerManager constructor called.");
    m_timers = new Timer*[maxNumTimers];
    for (uint32_t i = 0; i < maxNumTimers; i++) {
        m_timers[i] = nullptr;
    }
    m_maxNumTimers = maxNumTimers;
    LOG_DBG("TimerManager constructor finished.");
}

TimerManager::~TimerManager() {
    // Free the memory allocated in constructor.
    delete[] m_timers;
}

void TimerManager::registerTimer(Timer& timer) {
    __ASSERT(m_numTimers < m_maxNumTimers, "Max number of timers of %u reached.", m_maxNumTimers);
    m_timers[m_numTimers] = &timer;
    m_numTimers++;
    // Make sure to set the isRegistered flag to true. This will prevent log warnings
    // if the timer is started when it is not registered with any timer managers.
    timer.setIsRegistered(true);
}

TimerManager::TimerExpiryInfo TimerManager::getNextExpiringTimer() {
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

} // namespace zct
