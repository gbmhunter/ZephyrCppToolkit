#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "App.hpp"
#include "PeripheralsMock.hpp"

LOG_MODULE_REGISTER(GpioExample, LOG_LEVEL_DBG);

int main() {

    // Create mock peripherals.
    PeripheralsMock peripherals;

    // Create app.
    App app(peripherals);
}