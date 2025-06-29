#include <zephyr/ztest.h>
#include <zephyr/kernel.h>

void test_main(void)
{
    // Run all tests once
    ztest_run_all(NULL, false, 1, 1);
}