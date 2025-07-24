/**************************************************************************************
 * @file IWatchdog.hpp
 * 
 * Interface for watchdog peripherals in the ZephyrCppToolkit library.
 *  
 **************************************************************************************
 * 
 * Copyright 2025 Electronic Timers Ltd.
 * 
 **************************************************************************************/

#pragma once

// System includes
#include <functional>
#include <cstdint>

namespace zct {

/**
 * @brief Interface for a hardware watchdog peripheral (not the software watchdog, this does not need to be mocked).
 * 
 * This class provides a C++ abstraction layer over Zephyr's watchdog API.
 * It follows the same patterns as other ZephyrCppToolkit peripheral interfaces.
 */
class IWatchdog {
public:
    /**
     * @brief Watchdog configuration options.
     */
    enum class Option : uint8_t {
        None = 0,
        PauseInSleep = 1,           ///< Pause timer during CPU sleep
        PauseHaltedByDebug = 2      ///< Pause timer when debugger halts CPU
    };

    /**
     * @brief Watchdog reset behavior flags.
     */
    enum class ResetFlag : uint8_t {
        None = 0,
        ResetCpuCore = 1,           ///< Reset CPU core on timeout
        ResetSoc = 2                ///< Reset entire SoC on timeout
    };

    /**
     * @brief Watchdog callback function type.
     * 
     * @param channelId The channel ID that timed out
     * @param userData User data passed during configuration
     */
    using CallbackFn = std::function<void(int channelId, void* userData)>;

    /**
     * @brief Constructor.
     * 
     * @param name Name of the watchdog instance for logging purposes
     */
    IWatchdog(const char* name);

    /**
     * @brief Virtual destructor.
     */
    virtual ~IWatchdog() = default;

    /**
     * @brief Install a watchdog timeout configuration.
     * 
     * @param timeoutMs Timeout value in milliseconds
     * @param callback Callback function to execute on timeout (optional)
     * @param userData User data to pass to callback (optional)
     * @param flags Reset behavior flags
     * @return Channel ID on success, negative error code on failure
     */
    virtual int installTimeout(uint32_t timeoutMs, 
                              CallbackFn callback = nullptr,
                              void* userData = nullptr,
                              ResetFlag flags = ResetFlag::ResetSoc) = 0;

    /**
     * @brief Setup the watchdog with global configuration.
     * 
     * @param options Global watchdog options
     * @return 0 on success, negative error code on failure
     */
    virtual int setup(Option options = Option::None) = 0;

    /**
     * @brief Feed (service) a watchdog channel to prevent timeout.
     * 
     * @param channelId Channel ID to feed
     * @return 0 on success, negative error code on failure
     */
    virtual int feed(int channelId) = 0;

    /**
     * @brief Disable the watchdog instance.
     * 
     * @return 0 on success, negative error code on failure
     */
    virtual int disable() = 0;

    /**
     * @brief Get the name of this watchdog instance.
     * 
     * @return Name string
     */
    const char* getName() const { return m_name; }

    /**
     * @brief Get the raw watchdog device pointer. Only valid on real implementations, mock implementations will return nullptr.
     * 
     * @return Raw watchdog device pointer. Returns nullptr on mock implementations.
     */
    virtual const struct device* getRawDevice() const = 0;

protected:
    const char* m_name;     ///< Name of this watchdog instance
};

/**
 * @brief Bitwise OR operator for watchdog options.
 */
constexpr IWatchdog::Option operator|(IWatchdog::Option lhs, IWatchdog::Option rhs) {
    return static_cast<IWatchdog::Option>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

/**
 * @brief Bitwise AND operator for watchdog options.
 */
constexpr IWatchdog::Option operator&(IWatchdog::Option lhs, IWatchdog::Option rhs) {
    return static_cast<IWatchdog::Option>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

} // namespace zct