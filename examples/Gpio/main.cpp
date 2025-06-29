#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit.hpp"

LOG_MODULE_REGISTER(GpioExample, LOG_LEVEL_DBG);

static const struct gpio_dt_spec l_testGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(test_gpios, test_gpio1), gpios);

int main() {
    zct::GpioReal myGpio("MyGpio", &l_testGpioSpec, zct::IGpio::Direction::Output);

    while (true) {
        myGpio.set(true);
        k_sleep(K_SECONDS(1));
        myGpio.set(false);
        k_sleep(K_SECONDS(1));
    }
}