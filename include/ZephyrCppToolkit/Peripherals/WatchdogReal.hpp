/**************************************************************************************
 * @file WatchdogReal.hpp
 * 
 * Real hardware implementation of the watchdog interface using Zephyr's watchdog API.
 *  
 **************************************************************************************
 * 
 * Copyright 2025 Electronic Timers Ltd.
 * 
 **************************************************************************************/

#pragma once

// 3rd party includes
#include <zephyr/drivers/watchdog.h>

// Local includes
#include "IWatchdog.hpp"

namespace zct {

/**
 * @brief Real hardware implementation of the watchdog interface.
 * 
 * This class provides a C++ wrapper around Zephyr's watchdog driver API.
 */
class WatchdogReal : public IWatchdog {
public:
    /**
     * @brief Constructor.
     * 
     * @param name Name of the watchdog instance for logging purposes
     * @param device Pointer to the Zephyr watchdog device
     */
    WatchdogReal(const char* name, const struct device* device);

    /**
     * @brief Destructor.
     */
    ~WatchdogReal();

    /**
     * @brief Install a watchdog timeout configuration.
     * 
     * @param timeoutMs Timeout value in milliseconds
     * @param callback Callback function to execute on timeout (optional)
     * @param userData User data to pass to callback (optional)
     * @param flags Reset behavior flags
     * @param options Configuration options
     * @return Channel ID on success, negative error code on failure
     */
    int installTimeout(uint32_t timeoutMs, 
                      CallbackFn callback = nullptr,
                      void* userData = nullptr,
                      ResetFlag flags = ResetFlag::ResetSoc) override;

    /**
     * @brief Setup the watchdog with global configuration.
     * 
     * @param options Global watchdog options
     * @return 0 on success, negative error code on failure
     */
    int setup(Option options = Option::None) override;

    /**
     * @brief Feed (service) a watchdog channel to prevent timeout.
     * 
     * @param channelId Channel ID to feed
     * @return 0 on success, negative error code on failure
     */
    int feed(int channelId) override;

    /**
     * @brief Disable the watchdog instance.
     * 
     * @return 0 on success, negative error code on failure
     */
    int disable() override;

    /**
     * @brief Get the raw watchdog device pointer.
     * 
     * @return Raw watchdog device pointer
     */
    const struct device* getRawDevice() const override;

private:
    /**
     * @brief Convert IWatchdog::Option to Zephyr wdt options.
     * 
     * @param options IWatchdog options
     * @return Zephyr wdt options
     */
    static uint8_t convertOptions(Option options);

    /**
     * @brief Convert IWatchdog::ResetFlag to Zephyr wdt flags.
     * 
     * @param flags IWatchdog reset flags
     * @return Zephyr wdt flags
     */
    static uint8_t convertResetFlags(ResetFlag flags);

    /**
     * @brief Static callback function for Zephyr WDT API.
     * 
     * @param dev Watchdog device
     * @param channelId Channel that timed out
     */
    static void staticCallback(const struct device* dev, int channelId);

    const struct device* m_device;      ///< Zephyr watchdog device
    bool m_isSetup;                     ///< Track if setup() has been called
    CallbackFn m_callback;              ///< User callback function
    void* m_userData;                   ///< User data for callback
};

} // namespace zct