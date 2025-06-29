#include <zephyr/ztest.h>

#include "Mutex.hpp"

namespace zct = zephyr_cpp_toolkit;

ZTEST_SUITE(MutexTests, NULL, NULL, NULL, NULL, NULL);

void test_mutex_create(void);

ZTEST(MutexTests, test_mutex_create)
{
    zct::Mutex mutex;

    // Try and lock the mutex
    auto lockGuard = zct::MutexLockGuard::create(mutex);
    zassert_true(lockGuard.has_value(), "Failed to lock the mutex.");

    // Try and lock the mutex again
    auto lockGuard2 = zct::MutexLockGuard::create(mutex);
    zassert_false(lockGuard2.has_value(), "Mutex should be locked.");
}