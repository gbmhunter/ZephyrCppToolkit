#pragma once

#include <zephyr/kernel.h>

namespace zephyr_cpp_toolkit {

// Forward declarations
class Mutex;

/** 
 * A RAII class that locks the mutex when constructed and unlocks it when destroyed.
 */
class MutexLockGuard {
public:
    MutexLockGuard(Mutex& mutex, k_timeout_t timeout = K_FOREVER);
    ~MutexLockGuard();
private:
    // The mutex this guard is locking/unlocking.
    Mutex& m_mutex;
};

class Mutex {
public:
    Mutex();
    ~Mutex();

    /**
     * @brief Create a lock guard for the mutex which locks the mutex when constructed and unlocks it when destroyed.
     * 
     * This is a RAII class that ensures the mutex is unlocked when it goes out of scope. You don't normally need to unlock the mutex manually.
     * 
     * @param timeout The max. time to wait to lock the mutex.
     * @return MutexLockGuard  The lock guard for the mutex.
     */
    MutexLockGuard lockGuard(k_timeout_t timeout = K_FOREVER);

    /**
     * @brief Get the underlying Zephyr mutex object.
     * 
     * It is not recommended to use this function. Only use as an escape hatch.
     * 
     * @return A pointer to the Zephyr mutex object.
     */
    struct k_mutex* getZephyrMutex() const;
private:
    struct k_mutex m_zephyrMutex;
};

} // namespace zephyr_cpp_toolkit