#pragma once

// #include <expected>
#include <zephyr/kernel.h>

#include <tl/expected.hpp>

namespace zephyr_cpp_toolkit {

// Forward declarations
class Mutex;

/** 
 * A RAII class that locks the mutex when constructed and unlocks it when destroyed.
 */
class MutexLockGuard {
public:

    ~MutexLockGuard();

    /**
     * Since we can't return an error code from the constructor, to create a lock guard use this static function.
     */
    static tl::expected<MutexLockGuard, int> create(Mutex& mutex, k_timeout_t timeout = K_FOREVER);
protected:
    /**
     * Constructor is hidden since we can't return an error code from the constructor. Use create() instead.
     */
    MutexLockGuard(Mutex& mutex);

    // The mutex this guard is locking/unlocking.
    Mutex& m_mutex;
};

class Mutex {
public:
    /**
     * @brief Construct a new mutex.
     * 
     * The mutex starts of in an unlocked state. Call lockGuard() to lock the mutex.
     */
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
    tl::expected<MutexLockGuard, int> lockGuard(k_timeout_t timeout = K_FOREVER);

    /**
     * @brief Get the underlying Zephyr mutex object.
     * 
     * It is not recommended to use this function. Only use as an escape hatch.
     * 
     * @return A pointer to the Zephyr mutex object.
     */
    struct k_mutex* getZephyrMutex() const;
protected:
    struct k_mutex m_zephyrMutex;
};

} // namespace zephyr_cpp_toolkit