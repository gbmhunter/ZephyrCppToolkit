#include "Mutex.hpp"

namespace zephyr_cpp_toolkit {

//================================================================================================//
// MutexLockGuard
//================================================================================================//

MutexLockGuard::MutexLockGuard(Mutex& mutex, k_timeout_t timeout, int& mutexRc)
    : m_mutex(mutex)
{
    // Try and lock the mutex
    int mutexRcTemp = k_mutex_lock(mutex.getZephyrMutex(), timeout);
    mutexRc = mutexRcTemp;
}

MutexLockGuard::~MutexLockGuard()
{
    // Guard is being destroyed, so the mutex is unlocked.
    int mutexRcTemp = k_mutex_unlock(m_mutex.getZephyrMutex());
}

// tl::expected<MutexLockGuard, int> MutexLockGuard::create(Mutex& mutex, k_timeout_t timeout)
// {
//     // Try and lock the mutex
//     printk("Calling k_mutex_lock. mutex: %p\n", mutex.getZephyrMutex());
//     int mutexRcTemp = k_mutex_lock(mutex.getZephyrMutex(), timeout);
//     printk("k_mutex_lock returned: %d\n", mutexRcTemp);
//     if (mutexRcTemp != 0) {
//         printk("Mutex lock failed with error code: %d\n", mutexRcTemp);
//         return tl::unexpected(mutexRcTemp);
//     }
//     // If we get here, the mutex was locked successfully.
//     return tl::expected<MutexLockGuard, int>(MutexLockGuard(mutex));
// }

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

// tl::expected<MutexLockGuard, int> Mutex::lockGuard(k_timeout_t timeout)
// {
//     // Create a lock guard, passing in this mutex
//     return MutexLockGuard::create(*this, timeout);
// }

struct k_mutex* Mutex::getZephyrMutex()
{
    return &m_zephyrMutex;
}

} // namespace zephyr_cpp_toolkit