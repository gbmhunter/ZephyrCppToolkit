#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "App.hpp"
#include "PeripheralsMock.hpp"

LOG_MODULE_REGISTER(MyTests, LOG_LEVEL_DBG);

ZTEST_SUITE(MyTests, NULL, NULL, NULL, NULL, NULL);

ZTEST(MyTests, realGpioOutputCanBeSet)
{
    // Create mock peripherals.
    PeripheralsMock peripherals;

    // Create app.
    App app(peripherals);

    // Wait for the app to start.
    k_sleep(K_SECONDS(1));

    // Toggle the input GPIO.
    zct::GpioMock& inputGpio = static_cast<zct::GpioMock&>(peripherals.getInputGpio());

    // Get time we make input active, some of the checks below are relative to this time.
    int64_t inputPinActiveStartTimeMs = k_uptime_get();

    inputGpio.mockSetInput(true);
    k_sleep(K_MSEC(10));

    zct::GpioMock& outputGpio = static_cast<zct::GpioMock&>(peripherals.getOutputGpio());
    zassert_true(outputGpio.get() == true, "Output GPIO should be high.");

    // Wait until almost 1 min is up to make sure the output is still active
    k_sleep(K_TIMEOUT_ABS_MS(inputPinActiveStartTimeMs + 1000*59));

    zassert_true(outputGpio.get() == true, "Output GPIO should be high.");

    // Now go to the 1 min and check that output is now inactive
    k_sleep(K_TIMEOUT_ABS_MS(inputPinActiveStartTimeMs + 1000*61));

    zassert_true(outputGpio.get() == false, "Output GPIO should be low.");
}