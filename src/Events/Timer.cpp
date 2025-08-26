
//================================================================================================//
// INCLUDES
//================================================================================================//

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "ZephyrCppToolkit/Events/Timer.hpp"
#include "ZephyrCppToolkit/Events/TimerManager.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_Timer, LOG_LEVEL_DBG);

Timer::Timer(const char* name, std::function<void()> expiryCallback) :
    m_name(name),
    m_expiryCallback(expiryCallback)
{
}

Timer::Timer(const char* name, std::function<void()> expiryCallback, TimerManager& timerManager) :
    m_name(name),
    m_expiryCallback(expiryCallback)
{
    // Register the timer with the timer manager
    timerManager.registerTimer(*this);
}

void Timer::start(int64_t period_ms) {
    // Convert ms to ticks
    start(period_ms, period_ms);
}

void Timer::start(int64_t startDuration_ms, int64_t period_ms) {
    LOG_MODULE_DECLARE(zct_Timer, ZCT_TIMER_LOG_LEVEL);
    __ASSERT_NO_MSG(startDuration_ms >= 0); // Start time can be 0, which means the timer will fire immediately. Can't be negative!
    __ASSERT_NO_MSG(period_ms >= -1); // Period can be -1, which means the timer will not repeat
    
    if (!this->m_isRegistered) {
        LOG_WRN("Timer \"%s\" is not registered with a timer manager. Expiry events will not be handled.", this->m_name);
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

void Timer::stop() {
    this->m_isRunning = false;
    this->period_ticks = -1;
    this->startTime_ticks = 0;
    this->nextExpiryTime_ticks = 0;
}

bool Timer::isRunning() const { 
    return this->m_isRunning; 
}

void Timer::updateAfterExpiry() {
    LOG_MODULE_DECLARE(zct_Timer, ZCT_TIMER_LOG_LEVEL);
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

int64_t Timer::getNextExpiryTimeTicks() const { 
    return this->nextExpiryTime_ticks; 
}

void Timer::setIsRegistered(bool isRegistered) { 
    this->m_isRegistered = isRegistered; 
}

bool Timer::getIsRegistered() const { 
    return this->m_isRegistered; 
}

void Timer::setExpiryCallback(std::function<void()> callback) { 
    m_expiryCallback = callback; 
}

const std::function<void()>& Timer::getExpiryCallback() const { 
    return m_expiryCallback; 
}

} // namespace zct
