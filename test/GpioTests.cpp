#include <zephyr/ztest.h>

#include "ZephyrCppToolkit/Peripherals/GpioMock.hpp"
#include "ZephyrCppToolkit/Peripherals/GpioReal.hpp"

ZTEST_SUITE(GpioTests, NULL, NULL, NULL, NULL, NULL);

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

ZTEST(GpioTests, mockGpioInterruptLevelToActive)
{
    zct::GpioMock gpio("MockGpio", zct::IGpio::Direction::Input);
    uint32_t interruptHandlerCallCount = 0;
    gpio.configureInterrupt(zct::IGpio::InterruptMode::LevelToActive, [&]() {
        interruptHandlerCallCount++;
    });
    gpio.mockSetInput(true);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should have been called.");
    gpio.mockSetInput(false);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should not have been called.");
}

ZTEST(GpioTests, mockGpioWorksWithActiveLowLogic)
{
    zct::GpioMock gpio("MockGpio", zct::IGpio::Direction::Input);

    uint32_t interruptHandlerCallCount = 0;
    gpio.configureInterrupt(zct::IGpio::InterruptMode::LevelToActive, [&]() {
        interruptHandlerCallCount++;
    });

    zassert_equal(interruptHandlerCallCount, 0, "Interrupt handler should not have been called.");

    // GPIO input starts out as 0V, inactive. Changing the logic mode should
    // preserve the physical value, not the logical. So after this call it should
    // still be 0V, but active.
    gpio.setLogicMode(zct::IGpio::LogicMode::ActiveLow);

    // Changing the logic mode should trigger an interrupt! Since it is now active.
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should have been called.");
    zassert_equal(gpio.get(), true, "GPIO should be active.");
    zassert_equal(gpio.getPhysical(), false, "GPIO should be physically low.");

    gpio.mockSetInputPhysical(true);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should not have been called.");
    zassert_equal(gpio.get(), false, "GPIO should be inactive.");
    zassert_equal(gpio.getPhysical(), true, "GPIO should be physically high.");
}

ZTEST(GpioTests, mockGpioInterruptLevelToInactive)
{
    zct::GpioMock gpio("MockGpio", zct::IGpio::Direction::Input);
    uint32_t interruptHandlerCallCount = 0;
    gpio.configureInterrupt(zct::IGpio::InterruptMode::LevelToInactive, [&]() {
        interruptHandlerCallCount++;
    });
    gpio.mockSetInput(true);
    zassert_equal(interruptHandlerCallCount, 0, "Interrupt handler should not have been called.");
    gpio.mockSetInput(false);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should have been called.");
    gpio.mockSetInput(true);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should not have been called.");
}

ZTEST(GpioTests, mockGpioInterruptCanBeDisabled)
{
    zct::GpioMock gpio("MockGpio", zct::IGpio::Direction::Input);
    uint32_t interruptHandlerCallCount = 0;
    gpio.configureInterrupt(zct::IGpio::InterruptMode::LevelToActive, [&]() {
        interruptHandlerCallCount++;
    });
    gpio.mockSetInput(true);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should have been called.");
    gpio.mockSetInput(false);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should not have been called.");

    // Now disable the interrupt
    gpio.configureInterrupt(zct::IGpio::InterruptMode::Disable, nullptr);
    gpio.mockSetInput(true);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should not have been called.");
    gpio.mockSetInput(false);
    zassert_equal(interruptHandlerCallCount, 1, "Interrupt handler should not have been called.");
}