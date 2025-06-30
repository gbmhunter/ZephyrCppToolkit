#include <type_traits>

#include <zephyr/ztest.h>

#include "ZephyrCppToolkit/Core/Mutex.hpp"

ZTEST_SUITE(MutexTests, NULL, NULL, NULL, NULL, NULL);

void test_mutex_create(void);

K_THREAD_STACK_DEFINE(thread_stack, 1024);

void threadMain(void* arg1, void* arg2, void* arg3)
{
    zct::Mutex* mutex = static_cast<zct::Mutex*>(arg1);
    bool* lock_result = static_cast<bool*>(arg2);
    struct k_sem* completion_sem = static_cast<struct k_sem*>(arg3);

    auto thread_lock_guard = mutex->lockGuard(K_NO_WAIT);
    *lock_result = thread_lock_guard.didGetLock();
    k_sem_give(completion_sem);
}

bool tryLockInAnotherThread(zct::Mutex& mutex)
{
    // To make sure the mutex is locked, we need to create another thread
    // to try and lock it
    struct k_thread thread;
    struct k_sem thread_completion_sem;
    k_sem_init(&thread_completion_sem, 0, 1); // Initial count 0, max count 1
    bool thread_lock_successful;

    k_thread_create(&thread, thread_stack, K_THREAD_STACK_SIZEOF(thread_stack),
                    threadMain, &mutex, &thread_lock_successful, &thread_completion_sem,
                    0, /* priority */
                    0, /* options */
                    K_NO_WAIT);

    // Wait for the spawned thread to attempt the lock and signal completion
    k_sem_take(&thread_completion_sem, K_FOREVER);
    k_thread_join(&thread, K_FOREVER);

    return !thread_lock_successful;
}

ZTEST(MutexTests, testMutexLockGuard)
{
    zct::Mutex mutex;

    // Scoped block to make sure the lockGuard goes out of scope below
    {
        // Try and lock the mutex in the main thread
        zct::MutexLockGuard lockGuard = mutex.lockGuard(K_MSEC(100));

        // Should have locked ok.
        zassert_true(lockGuard.didGetLock(), "Failed to lock the mutex in main thread.");

        // Assert that the spawned thread failed to lock the mutex
        zassert_true(tryLockInAnotherThread(mutex), "Spawned thread should have failed to lock the mutex.");
    } // lockGuard goes out of scope, so the mutex is unlocked

    // Now make sure the mutex has been unlocked
    zassert_false(tryLockInAnotherThread(mutex), "Mutex should be unlocked after lockGuard goes out of scope.");
}

// Make sure we can't copy or move the lock guard.
static_assert(!std::is_copy_constructible_v<zct::MutexLockGuard>);
