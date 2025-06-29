#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/PwmMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_PwmMock, LOG_LEVEL_WRN);

PwmMock::PwmMock(const char* name)
    :
    IPwm(name)
{}

void PwmMock::set(uint32_t periodNs, uint32_t pulseWidthNs) {
    LOG_DBG("Setting PWM period to %u ns and pulse width to %u ns.", periodNs, pulseWidthNs);
    // Do nothing
}

} // namespace zct