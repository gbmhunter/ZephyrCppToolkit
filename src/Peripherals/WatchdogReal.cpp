/**************************************************************************************
 * @file WatchdogReal.cpp
 * 
 * Implementation of the real hardware watchdog class.
 *  
 **************************************************************************************
 * 
 * Copyright 2025 Electronic Timers Ltd.
 * 
 **************************************************************************************/

// 3rd party includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>

// Local includes
#include "ZephyrCppToolkit/Peripherals/WatchdogReal.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_WatchdogReal, LOG_LEVEL_WRN);

// Static instance pointer for callback routing
static WatchdogReal* s_instance = nullptr;

WatchdogReal::WatchdogReal(const char* name, const struct device* device) :
    IWatchdog(name),
    m_device(device),
    m_isSetup(false),
    m_callback(nullptr),
    m_userData(nullptr)
{
    __ASSERT_NO_MSG(device != nullptr);
    __ASSERT(device_is_ready(device), "Watchdog device '%s' is not ready", device->name);
    
    // Store instance for static callback routing
    s_instance = this;
    
    LOG_DBG("WatchdogReal '%s' created with device '%s'.", m_name, device->name);
}

WatchdogReal::~WatchdogReal()
{
    LOG_DBG("WatchdogReal '%s' destroyed.", m_name);
    s_instance = nullptr;
}

int WatchdogReal::installTimeout(uint32_t timeoutMs, 
                                CallbackFn callback,
                                void* userData,
                                ResetFlag flags)
{
    LOG_DBG("WatchdogReal '%s': Installing timeout of %u ms.", m_name, timeoutMs);
    
    // Store callback and user data
    m_callback = callback;
    m_userData = userData;
    
    // Configure timeout structure
    struct wdt_timeout_cfg cfg = {
        .window = {
            .min = 0,
            .max = timeoutMs
        },
        .callback = (callback != nullptr) ? staticCallback : nullptr,
        .flags = convertResetFlags(flags)
    };
    
    int channelId = wdt_install_timeout(m_device, &cfg);
    
    if (channelId >= 0) {
        LOG_DBG("WatchdogReal '%s': Timeout installed successfully, channel ID: %d.", m_name, channelId);
    } else {
        LOG_ERR("WatchdogReal '%s': Failed to install timeout, error: %d.", m_name, channelId);
    }
    
    return channelId;
}

int WatchdogReal::setup(Option options)
{
    LOG_DBG("WatchdogReal '%s': Setting up watchdog.", m_name);
    
    uint8_t zephyrOptions = convertOptions(options);
    int result = wdt_setup(m_device, zephyrOptions);
    
    if (result == 0) {
        m_isSetup = true;
        LOG_DBG("WatchdogReal '%s': Setup completed successfully.", m_name);
    } else {
        LOG_ERR("WatchdogReal '%s': Setup failed with error: %d.", m_name, result);
    }
    
    return result;
}

int WatchdogReal::feed(int channelId)
{
    LOG_DBG("WatchdogReal '%s': Feeding channel %d.", m_name, channelId);
    
    int result = wdt_feed(m_device, channelId);
    
    if (result != 0) {
        LOG_ERR("WatchdogReal '%s': Failed to feed channel %d, error: %d.", m_name, channelId, result);
    }
    
    return result;
}

int WatchdogReal::disable()
{
    LOG_DBG("WatchdogReal '%s': Disabling watchdog.", m_name);
    
    int result = wdt_disable(m_device);
    
    if (result == 0) {
        m_isSetup = false;
        LOG_DBG("WatchdogReal '%s': Disabled successfully.", m_name);
    } else {
        LOG_ERR("WatchdogReal '%s': Failed to disable, error: %d.", m_name, result);
    }
    
    return result;
}

uint8_t WatchdogReal::convertOptions(Option options)
{
    uint8_t zephyrOptions = 0;
    
    if ((options & Option::PauseInSleep) != Option::None) {
        zephyrOptions |= WDT_OPT_PAUSE_IN_SLEEP;
    }
    
    if ((options & Option::PauseHaltedByDebug) != Option::None) {
        zephyrOptions |= WDT_OPT_PAUSE_HALTED_BY_DBG;
    }
    
    return zephyrOptions;
}

uint8_t WatchdogReal::convertResetFlags(ResetFlag flags)
{
    uint8_t zephyrFlags = 0;
    
    switch (flags) {
        case ResetFlag::None:
            zephyrFlags = WDT_FLAG_RESET_NONE;
            break;
        case ResetFlag::ResetCpuCore:
            zephyrFlags = WDT_FLAG_RESET_CPU_CORE;
            break;
        case ResetFlag::ResetSoc:
            zephyrFlags = WDT_FLAG_RESET_SOC;
            break;
    }
    
    return zephyrFlags;
}

void WatchdogReal::staticCallback(const struct device* dev, int channelId)
{
    if (s_instance != nullptr && s_instance->m_callback != nullptr) {
        LOG_DBG("WatchdogReal '%s': Timeout callback triggered for channel %d.", s_instance->m_name, channelId);
        s_instance->m_callback(channelId, s_instance->m_userData);
    }
}

const struct device* WatchdogReal::getRawDevice() const
{
    return m_device;
}

} // namespace zct