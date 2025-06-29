#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/PwmReal.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_PwmReal, LOG_LEVEL_WRN);

PwmReal::PwmReal(const char* name, const struct pwm_dt_spec& pwmDtSpec)
    :
    IPwm(name),
    m_pwmDtSpec(pwmDtSpec)
{}

void PwmReal::set(uint32_t periodNs, uint32_t pulseWidthNs) {
    LOG_DBG("Setting PWM period to %u ns and pulse width to %u ns.", periodNs, pulseWidthNs);
    // Call Zephyr API
    pwm_set_dt(&m_pwmDtSpec, periodNs, pulseWidthNs);
}

} // namespace zct