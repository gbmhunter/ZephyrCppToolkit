#include <zephyr/ztest.h>

#include "ZephyrCppToolkit.hpp"

ZTEST_SUITE(GpioTests, NULL, NULL, NULL, NULL, NULL);

static const struct gpio_dt_spec l_myGpioSpec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static const struct gpio_dt_spec l_testGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(test_gpios, test_gpio1), gpios);


ZTEST(GpioTests, realGpioOutputCanBeSet)
{
    zct::GpioReal gpio("MyGpio", &l_testGpioSpec, zct::IGpio::Direction::Output);
    gpio.set(true);

    zassert_true(gpio.get() == true, "GPIO should be high.");
}

ZTEST(GpioTests, mockGpioOutputCanBeSet)
{
    zct::GpioMock gpio("MockGpio", zct::IGpio::Direction::Output);
    // Should default to inactive
    zassert_true(gpio.get() == false, "GPIO should be inactive.");
    gpio.set(true);
    zassert_true(gpio.get() == true, "GPIO should be active.");
    gpio.set(false);
    zassert_true(gpio.get() == false, "GPIO should be inactive.");
}