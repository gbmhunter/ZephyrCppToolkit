#pragma once

#include <zephyr/kernel.h>

namespace zct {

// Forward declarations
class Mutex;

/** 
 * A RAII class that locks the mutex when constructed and unlocks it when destroyed.
 */
class MutexLockGuard {
public:
    /**
     * \brief Constructor is hidden since we can't return an error code from the constructor. Use create() instead.
     */
    MutexLockGuard(Mutex& mutex, k_timeout_t timeout, int& mutexRc);

    ~MutexLockGuard();

    // Prevent copying and moving
    MutexLockGuard(const MutexLockGuard&) = delete;
    MutexLockGuard& operator=(const MutexLockGuard&) = delete;
    MutexLockGuard(MutexLockGuard&&) = delete;
    MutexLockGuard& operator=(MutexLockGuard&&) = delete;
protected:
    

    // The mutex this guard is locking/unlocking.
    Mutex& m_mutex;

    bool m_didGetLock = false;
};

/**
 * Mutex is a C++ wrapper around a Zephyr mutex.
 * 
 * The recommended way to lock a mutex is to use the MutexLockGuard class.
 *
 * \sa MutexLockGuard
 */
class Mutex {
public:
    /**
     * @brief Construct a new mutex.
     * 
     * The mutex starts of in an unlocked state. Use a MutexLockGuard to lock the mutex.
     */
    Mutex();

    /**
     * @brief Destroy the mutex.
     */
    ~Mutex();

    /**
     * @brief Get the underlying Zephyr mutex object.
     * 
     * It is not recommended to use this function. Only use as an escape hatch if
     * the provided C++ API is not sufficient.
     * 
     * @return A pointer to the Zephyr mutex object.
     */
    struct k_mutex* getZephyrMutex();
protected:

    /**
     * The underlying Zephyr mutex object. Get access to this by calling getZephyrMutex().
     */
    struct k_mutex m_zephyrMutex;
};

} // namespace zct