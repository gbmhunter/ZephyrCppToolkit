#pragma once

#include <zephyr/kernel.h>
#include <tl/expected.hpp>

namespace zct {

// Forward declarations
class Mutex;

/** 
 * A RAII class that locks the mutex when constructed and unlocks it when destroyed.
 */
class MutexLockGuard {
public:

    ~MutexLockGuard();

    // Prevent copying and moving
    MutexLockGuard(const MutexLockGuard&) = delete;
    MutexLockGuard& operator=(const MutexLockGuard&) = delete;
    MutexLockGuard(MutexLockGuard&&);
    MutexLockGuard& operator=(MutexLockGuard&&) = delete;

    /**
     * \brief Check if the mutex was successfully locked with this lock guard.
     * 
     * \return True if the mutex was successfully locked with this lock guard, false otherwise.
     */
    bool didGetLock() const;

    // Allow Mutex to construct MutexLockGuard objects
    friend class Mutex;

protected:
    /**
     * \brief Constructor is hidden since we only want to allow the Mutex class to construct MutexLockGuard objects.
     */
    MutexLockGuard(Mutex& mutex, k_timeout_t timeout, int& mutexRc);

    /**
     * The mutex this guard is locking/unlocking.
     */
    Mutex& m_mutex;

    bool m_didGetLock = false;
};

/**
 * Mutex is a C++ wrapper around a Zephyr mutex.
 * 
 * The recommended way to lock a mutex is to use the lockGuard() function which returns a MutexLockGuard object. This will automatically unlock the mutex when the lock guard goes out of scope.
 * 
 * Just like with the Zephyr mutex, they are not designed for use in interrupts.
 *
 * \sa MutexLockGuard
 * 
 * Below is an example of how to use the Mutex class:
 * \include /examples/Mutex/main.cpp
 */
class Mutex {
public:
    /**
     * \brief Construct a new mutex.
     * 
     * The mutex starts of in an unlocked state. Use a MutexLockGuard to lock the mutex.
     */
    Mutex();

    /**
     * \brief Destroy the mutex.
     */
    ~Mutex();

    /**
     * \brief Get a lock guard for this mutex.
     * 
     * Note that this function may fail to lock the mutex. It is the callers responsibility to check if the lock was successful with didGetLock() after making this call.
     * 
     * \param timeout The timeout for the lock. Pass K_FOREVER to wait indefinitely, or K_NO_WAIT to not wait at all.
     * \return A lock guard for this mutex.
     */
    MutexLockGuard lockGuard(k_timeout_t timeout);

    /**
     * \brief Get the underlying Zephyr mutex object.
     * 
     * It is not recommended to use this function. Only use as an escape hatch if
     * the provided C++ API is not sufficient.
     * 
     * \return A pointer to the Zephyr mutex object.
     */
    struct k_mutex* getZephyrMutex();
protected:

    /**
     * The underlying Zephyr mutex object. Get access to this by calling getZephyrMutex().
     */
    struct k_mutex m_zephyrMutex;
};

} // namespace zct