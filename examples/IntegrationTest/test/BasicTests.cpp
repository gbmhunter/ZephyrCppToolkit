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
    inputGpio.mockSetInput(true);
    k_sleep(K_SECONDS(1));

    zct::GpioMock& outputGpio = static_cast<zct::GpioMock&>(peripherals.getOutputGpio());
    zassert_true(outputGpio.get() == true, "Output GPIO should be high.");

    LOG_WRN("Test finished.");
}