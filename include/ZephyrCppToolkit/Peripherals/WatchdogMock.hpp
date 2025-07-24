/**************************************************************************************
 * @file WatchdogMock.hpp
 * 
 * Mock implementation of the watchdog interface for testing purposes.
 *  
 **************************************************************************************
 * 
 * Copyright 2025 Electronic Timers Ltd.
 * 
 **************************************************************************************/

#pragma once

// System includes
#include <vector>
#include <chrono>

// Local includes
#include "IWatchdog.hpp"

namespace zct {

/**
 * @brief Mock implementation of the watchdog interface for testing.
 * 
 * This class simulates watchdog behavior in software for unit testing purposes.
 */
class WatchdogMock : public IWatchdog {
public:
    /**
     * @brief Structure to hold information about an installed timeout channel.
     */
    struct TimeoutChannel {
        uint32_t timeoutMs;         ///< Timeout value in milliseconds
        CallbackFn callback;        ///< Callback function
        void* userData;             ///< User data for callback
        ResetFlag flags;            ///< Reset behavior flags
        bool isActive;              ///< Whether the channel is active
        std::chrono::steady_clock::time_point lastFed;  ///< Last time channel was fed
    };

    /**
     * @brief Constructor.
     * 
     * @param name Name of the watchdog instance for logging purposes
     */
    WatchdogMock(const char* name);

    /**
     * @brief Destructor.
     */
    ~WatchdogMock();

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

    // ========================================
    // Mock-specific methods for testing
    // ========================================

    /**
     * @brief Manually trigger a timeout for a specific channel.
     * 
     * @param channelId Channel ID to trigger timeout for
     */
    void mockTriggerTimeout(int channelId);

    /**
     * @brief Check if the watchdog has been set up.
     * 
     * @return True if setup() has been called successfully
     */
    bool mockIsSetup() const { return m_isSetup; }

    /**
     * @brief Check if the watchdog is disabled.
     * 
     * @return True if disable() has been called
     */
    bool mockIsDisabled() const { return m_isDisabled; }

    /**
     * @brief Get the number of installed timeout channels.
     * 
     * @return Number of channels
     */
    size_t mockGetChannelCount() const { return m_channels.size(); }

    /**
     * @brief Get information about a specific channel.
     * 
     * @param channelId Channel ID to query
     * @return Pointer to channel info, or nullptr if channel doesn't exist
     */
    const TimeoutChannel* mockGetChannelInfo(int channelId) const;

    /**
     * @brief Check if a channel is expired (past its timeout).
     * 
     * @param channelId Channel ID to check
     * @return True if channel has expired
     */
    bool mockIsChannelExpired(int channelId) const;

    /**
     * @brief Get the time remaining until timeout for a channel.
     * 
     * @param channelId Channel ID to check
     * @return Time remaining in milliseconds, or -1 if channel doesn't exist
     */
    int64_t mockGetTimeRemainingMs(int channelId) const;

    /**
     * @brief Get the number of times a channel has been fed.
     * 
     * @param channelId Channel ID to check
     * @return Feed count for the channel
     */
    uint32_t mockGetFeedCount(int channelId) const;

    /**
     * @brief Reset all mock state (useful for test setup).
     */
    void mockReset();

private:
    std::vector<TimeoutChannel> m_channels;     ///< Installed timeout channels
    std::vector<uint32_t> m_feedCounts;        ///< Feed count per channel
    bool m_isSetup;                             ///< Whether setup() has been called
    bool m_isDisabled;                          ///< Whether disable() has been called
    Option m_globalOptions;                     ///< Global watchdog options
    int m_nextChannelId;                        ///< Next channel ID to assign
};

} // namespace zct