/**************************************************************************************
 * @file WatchdogMock.cpp
 * 
 * Implementation of the mock watchdog class for testing.
 *  
 **************************************************************************************
 * 
 * Copyright 2025 Electronic Timers Ltd.
 * 
 **************************************************************************************/

// 3rd party includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// Local includes
#include "ZephyrCppToolkit/Peripherals/WatchdogMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(WatchdogMock, LOG_LEVEL_WRN);

WatchdogMock::WatchdogMock(const char* name) :
    IWatchdog(name),
    m_isSetup(false),
    m_isDisabled(false),
    m_globalOptions(Option::None),
    m_nextChannelId(0)
{
    LOG_DBG("WatchdogMock '%s' created.", m_name);
}

WatchdogMock::~WatchdogMock()
{
    LOG_DBG("WatchdogMock '%s' destroyed.", m_name);
}

int WatchdogMock::installTimeout(uint32_t timeoutMs, 
                                CallbackFn callback,
                                void* userData,
                                ResetFlag flags)
{
    LOG_DBG("WatchdogMock '%s': Installing timeout of %u ms.", m_name, timeoutMs);
    
    if (m_isDisabled) {
        LOG_ERR("WatchdogMock '%s': Cannot install timeout on disabled watchdog.", m_name);
        return -EINVAL;
    }
    
    TimeoutChannel channel = {
        .timeoutMs = timeoutMs,
        .callback = callback,
        .userData = userData,
        .flags = flags,
        .isActive = false,
        .lastFed = std::chrono::steady_clock::now()
    };
    
    int channelId = m_nextChannelId++;
    
    // Ensure vectors are large enough
    if (static_cast<size_t>(channelId) >= m_channels.size()) {
        m_channels.resize(channelId + 1);
        m_feedCounts.resize(channelId + 1, 0);
    }
    
    m_channels[channelId] = channel;
    
    LOG_DBG("WatchdogMock '%s': Timeout installed successfully, channel ID: %d.", m_name, channelId);
    return channelId;
}

int WatchdogMock::setup(Option options)
{
    LOG_DBG("WatchdogMock '%s': Setting up watchdog.", m_name);
    
    if (m_isDisabled) {
        LOG_ERR("WatchdogMock '%s': Cannot setup disabled watchdog.", m_name);
        return -EINVAL;
    }
    
    m_globalOptions = options;
    m_isSetup = true;
    
    // Activate all installed channels
    for (auto& channel : m_channels) {
        channel.isActive = true;
        channel.lastFed = std::chrono::steady_clock::now();
    }
    
    LOG_DBG("WatchdogMock '%s': Setup completed successfully.", m_name);
    return 0;
}

int WatchdogMock::feed(int channelId)
{
    LOG_DBG("WatchdogMock '%s': Feeding channel %d.", m_name, channelId);
    
    if (m_isDisabled) {
        LOG_ERR("WatchdogMock '%s': Cannot feed disabled watchdog.", m_name);
        return -EINVAL;
    }
    
    if (!m_isSetup) {
        LOG_ERR("WatchdogMock '%s': Cannot feed watchdog that hasn't been setup.", m_name);
        return -EINVAL;
    }
    
    if (channelId < 0 || static_cast<size_t>(channelId) >= m_channels.size()) {
        LOG_ERR("WatchdogMock '%s': Invalid channel ID %d.", m_name, channelId);
        return -EINVAL;
    }
    
    if (!m_channels[channelId].isActive) {
        LOG_ERR("WatchdogMock '%s': Channel %d is not active.", m_name, channelId);
        return -EINVAL;
    }
    
    m_channels[channelId].lastFed = std::chrono::steady_clock::now();
    m_feedCounts[channelId]++;
    
    LOG_DBG("WatchdogMock '%s': Channel %d fed successfully (feed count: %u).", 
            m_name, channelId, m_feedCounts[channelId]);
    return 0;
}

int WatchdogMock::disable()
{
    LOG_DBG("WatchdogMock '%s': Disabling watchdog.", m_name);
    
    m_isDisabled = true;
    m_isSetup = false;
    
    // Deactivate all channels
    for (auto& channel : m_channels) {
        channel.isActive = false;
    }
    
    LOG_DBG("WatchdogMock '%s': Disabled successfully.", m_name);
    return 0;
}

const struct device* WatchdogMock::getRawDevice() const
{
    // Returns nullptr on mock implementations
    return nullptr;
}

void WatchdogMock::mockTriggerTimeout(int channelId)
{
    LOG_DBG("WatchdogMock '%s': Manually triggering timeout for channel %d.", m_name, channelId);
    
    if (channelId < 0 || static_cast<size_t>(channelId) >= m_channels.size()) {
        LOG_ERR("WatchdogMock '%s': Invalid channel ID %d for timeout trigger.", m_name, channelId);
        return;
    }
    
    const auto& channel = m_channels[channelId];
    if (channel.callback != nullptr) {
        LOG_DBG("WatchdogMock '%s': Executing timeout callback for channel %d.", m_name, channelId);
        channel.callback(channelId, channel.userData);
    }
}

const WatchdogMock::TimeoutChannel* WatchdogMock::mockGetChannelInfo(int channelId) const
{
    if (channelId < 0 || static_cast<size_t>(channelId) >= m_channels.size()) {
        return nullptr;
    }
    
    return &m_channels[channelId];
}

bool WatchdogMock::mockIsChannelExpired(int channelId) const
{
    const auto* channel = mockGetChannelInfo(channelId);
    if (channel == nullptr || !channel->isActive) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - channel->lastFed);
    
    return elapsed.count() >= static_cast<int64_t>(channel->timeoutMs);
}

int64_t WatchdogMock::mockGetTimeRemainingMs(int channelId) const
{
    const auto* channel = mockGetChannelInfo(channelId);
    if (channel == nullptr || !channel->isActive) {
        return -1;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - channel->lastFed);
    
    int64_t remaining = static_cast<int64_t>(channel->timeoutMs) - elapsed.count();
    return (remaining > 0) ? remaining : 0;
}

uint32_t WatchdogMock::mockGetFeedCount(int channelId) const
{
    if (channelId < 0 || static_cast<size_t>(channelId) >= m_feedCounts.size()) {
        return 0;
    }
    
    return m_feedCounts[channelId];
}

void WatchdogMock::mockReset()
{
    LOG_DBG("WatchdogMock '%s': Resetting all mock state.", m_name);
    
    m_channels.clear();
    m_feedCounts.clear();
    m_isSetup = false;
    m_isDisabled = false;
    m_globalOptions = Option::None;
    m_nextChannelId = 0;
}

} // namespace zct