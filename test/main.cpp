#include <zephyr/ztest.h>
#include <zephyr/kernel.h>

void test_main(void)
{
    struct k_mutex m_mutex;
    k_mutex_init(&m_mutex);
    // Run all tests once
    ztest_run_all(NULL, false, 1, 1);
}