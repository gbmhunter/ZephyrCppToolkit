#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit.hpp"

LOG_MODULE_REGISTER(GpioExample, LOG_LEVEL_DBG);

static const struct gpio_dt_spec l_inputGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(example_gpios, input_gpio), gpios);
static const struct gpio_dt_spec l_outputGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(example_gpios, output_gpio), gpios);

int main() {

    // Create an input GPIO. By default they are set to be an input, active high.
    // The name is used for logging purposes.
    zct::GpioReal myInput("MyInput", &l_inputGpioSpec);

    // Create an output GPIO.
    zct::GpioReal myOutput("MyOutput", &l_outputGpioSpec, zct::IGpio::Direction::Output);

    while (true) {
        // Read logical value of input GPIO.
        bool inputValue = myInput.get();
        LOG_INF("Input value: %d", inputValue);

        // Set logical value of output GPIO.
        myOutput.set(!inputValue);
    }
}