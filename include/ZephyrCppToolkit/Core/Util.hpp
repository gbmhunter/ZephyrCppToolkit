#pragma once

#include <cstdint>

#include <zephyr/kernel.h>

namespace zct {

class Util {
public:
    /**
     * Sleep until the system time is equal to or greater than the target time.
     * This is useful in testing when we want to sleep until a specific absolute time,
     * rather than a relative time as per k_sleep(K_MSEC(...)). Uses k_sleep() to sleep.
     * 
     * Typical use would be:
     * \code
     * int64_t startTimeMs = k_uptime_get();
     * zct::Util::sleepUntilSystemTime(startTimeMs + 1000);
     * \endcode
     * 
     * \param targetTimeMs The target time (as a system time in milliseconds) to sleep until.
     */
    static void sleepUntilSystemTime(int64_t targetTimeMs)
    {
        int64_t currentTimeMs = k_uptime_get();
        int64_t remainingTimeMs = targetTimeMs - currentTimeMs;
        
        if (remainingTimeMs > 0) {
            k_sleep(K_MSEC(remainingTimeMs));
        }
    }
};

} // namespace zct