#include <zephyr/ztest.h>

#include "Mutex.hpp"

ZTEST_SUITE(MutexTests, NULL, NULL, NULL, NULL, NULL);

void test_mutex_create(void);

ZTEST(MutexTests, test_mutex_create)
{
    zephyr_cpp_toolkit::Mutex mutex;
}