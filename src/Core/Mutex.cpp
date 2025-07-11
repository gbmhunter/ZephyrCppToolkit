#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Core/Mutex.hpp"

LOG_MODULE_REGISTER(zct_Mutex, LOG_LEVEL_DBG);

namespace zct {


//================================================================================================//
// MutexLockGuard
//================================================================================================//

MutexLockGuard::MutexLockGuard(Mutex& mutex, k_timeout_t timeout, int& mutexRc)
    : m_mutex(mutex)
{
    // Try and lock the mutex
    LOG_DBG("Locking mutex: %p", mutex.getZephyrMutex());
    int mutexRcTemp = k_mutex_lock(mutex.getZephyrMutex(), timeout);
    LOG_DBG("k_mutex_lock returned: %d", mutexRcTemp);
    if (mutexRcTemp == 0) {
        m_didGetLock = true;
    }
    mutexRc = mutexRcTemp;
}

MutexLockGuard::~MutexLockGuard()
{
    LOG_DBG("MutexLockGuard destructor called. m_didGetLock: %d", m_didGetLock);
    // Guard is being destroyed. Check if we managed to get a lock, and if so, unlock the
    // mutex.
    if (m_didGetLock) {
        LOG_DBG("Unlocking mutex: %p", m_mutex.getZephyrMutex());
        int mutexRc = k_mutex_unlock(m_mutex.getZephyrMutex());
        LOG_DBG("k_mutex_unlock returned: %d", mutexRc);
        __ASSERT_NO_MSG(mutexRc == 0);
    }
}

bool MutexLockGuard::didGetLock() const
{
    return m_didGetLock;
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
    LOG_DBG("%s() called on mutex %p", __func__, getZephyrMutex());
    // Create a lock guard, passing in this mutex
    int mutexRc;
    return MutexLockGuard(*this, timeout, mutexRc);
}

struct k_mutex* Mutex::getZephyrMutex()
{
    return &m_zephyrMutex;
}

} // namespace zct