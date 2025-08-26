#pragma once

//================================================================================================//
// INCLUDES
//================================================================================================//

#include <cstdint>
#include <cstddef>
#include <functional>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

//================================================================================================//
// MACROS
//================================================================================================//

// Don't use constexpr here, it seg faults!
// static constexpr int LOG_LEVEL = LOG_LEVEL_DBG;
#define ZCT_TIMER_LOG_LEVEL LOG_LEVEL_WRN

namespace zct {

// Forward declarations
class TimerManager;

//================================================================================================//
// CLASS DECLARATION
//================================================================================================//

/**
 * \brief A timer that can be used to execute callbacks at regular intervals in an event driven application.
 * 
 * This class is designed to be used with the zct::TimerManager class. The timer manager
 * monitors a list of timers and executes callbacks when the timers expire.
 * 
 * This class is also designed to be used with the zct::EventThread class. The event thread
 * can block until either a timer expires or an external event is received.
 */
class Timer {
public:

    /**
     * Create a new timer with a callback function.
     * 
     * The timer will not be running after creation.
     * 
     * \param name The name of the timer, used for logging purposes.
     * \param expiryCallback Callback function to call when the timer expires. This will be
     *                       called by the EventThread when the timer expires.
     */
    Timer(const char* name, std::function<void()> expiryCallback);

    /**
     * Create a new timer with a callback function and automatically register it with a timer manager.
     * 
     * This is the recommended way to create a timer, as there is no chance to forget to register the timer with a timer manager.
     * 
     * The timer will not be running after creation.
     * 
     * \param name The name of the timer, used for logging purposes.
     * \param expiryCallback Callback function to call when the timer expires. This will be
     *                       called by the EventThread when the timer expires.
     * \param timerManager The timer manager to register the timer with.
     */
    Timer(const char* name, std::function<void()> expiryCallback, TimerManager& timerManager);

    /**
     * Start the timer in reoccurring mode. The timer will expire for the first time
     * after period_ms from when this is called, and then period_ms after that.
     * 
     * @param period_ms The period of the timer. Should be a positive integer.
    */
    void start(int64_t period_ms);

    /**
     * Start the timer in either one-shot or reoccurring mode.
     * 
     * @param startDuration_ms The time to wait before the first expiry. Must either be 0 (no-wait) or positive.
     * @param period_ms The period of the timer. Set to -1 for a one-shot timer, or 0/positive for a recurring timer.
    */
    void start(int64_t startDuration_ms, int64_t period_ms);

    /**
     * Stop the timer. This will prevent the timer from expiring until
     * start() is called again.
     * 
     * This does not deregister the timer from the timer manager (if registered), nor
     * does this clear the event that is saved in the timer for when it expires.
     */
    void stop();

    /**
     * Check if the timer is running.
     * 
     * @return true if the timer is running, false otherwise.
    */
    bool isRunning() const;

    /**
     * Designed to be called by the state machine when the timer expires.
     * 
     * This will either:
     * 1. Stop the timer if it is a one-shot timer.
     * 2. Update the next expiry time if it is a recurring timer.
    */
    void updateAfterExpiry();


    /**
     * Get the next expiry time of the timer.
     * 
     * @return The next expiry time of the timer in ticks.
     */
    int64_t getNextExpiryTimeTicks() const;

    /**
     * Set the isRegistered flag. Used to log warnings if a timer is
     * started when it is not registered with any timer managers.
     * 
     * @param isRegistered The value to set the isRegistered flag to.
     */
    void setIsRegistered(bool isRegistered);

    /**
     * Get the isRegistered flag.
     * 
     * @return The value of the isRegistered flag.
     */
    bool getIsRegistered() const;

    /**
     * Set the expiry callback function.
     * 
     * @param callback The callback function to call when the timer expires. Set to nullptr to disable callback.
     */
    void setExpiryCallback(std::function<void()> callback);

    /**
     * Get the expiry callback function.
     * 
     * @return The expiry callback function, or nullptr if no callback is set.
     */
    const std::function<void()>& getExpiryCallback() const;

protected:
    int64_t period_ticks = 0;
    int64_t startTime_ticks = 0;
    int64_t nextExpiryTime_ticks = 0;
    bool m_isRunning = false;
    bool m_isRegistered = false;
    const char* m_name;
    std::function<void()> m_expiryCallback;
};

} // namespace zct