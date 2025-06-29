#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/GpioMock.hpp"

LOG_MODULE_REGISTER(GpioExample, LOG_LEVEL_DBG);

// static const struct gpio_dt_spec l_inputGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(example_gpios, input_gpio), gpios);
// static const struct gpio_dt_spec l_outputGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(example_gpios, output_gpio), gpios);

int main() {

    // Create an input GPIO. By default they are set to be an input, active high.
    // The name is used for logging purposes.
    zct::GpioMock myInput("MyInput");

    // Create an output GPIO.
    zct::GpioMock myOutput("MyOutput", zct::IGpio::Direction::Output);

    while (true) {
        // Read logical value of input GPIO.
        bool inputValue = myInput.get();
        LOG_INF("Input value: %d", inputValue);

        // Configure interrupt on our input GPIO
        myInput.configureInterrupt(zct::IGpio::InterruptMode::LevelToActive, []() {
            // WARNING: This is called from an interrupt context
            LOG_INF("Interrupt occurred");
        });

        // Set logical value of output GPIO.
        myOutput.set(!inputValue);
    }
}