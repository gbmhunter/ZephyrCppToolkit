#include "Mutex.hpp"

namespace zephyr_cpp_toolkit {

//================================================================================================//
// MutexLockGuard
//================================================================================================//

MutexLockGuard::MutexLockGuard(Mutex& mutex, k_timeout_t timeout)
    : m_mutex(mutex)
{
    // The mutex is locked when the lock guard is constructed.
    k_mutex_lock(m_mutex.getZephyrMutex(), timeout);
}

MutexLockGuard::~MutexLockGuard()
{
    // Guard is being destroyed, so the mutex is unlocked.
    k_mutex_unlock(m_mutex.getZephyrMutex());
}

//================================================================================================//
// Mutex
//================================================================================================//

Mutex::Mutex()
{
    // Create Zephyr mutex
    k_mutex_init(&m_zephyrMutex);
}

Mutex::~Mutex()
{
}

MutexLockGuard Mutex::lockGuard(k_timeout_t timeout)
{
    // Create a lock guard, passing in this mutex
    return MutexLockGuard(*this, timeout);
}

} // namespace zephyr_cpp_toolkit