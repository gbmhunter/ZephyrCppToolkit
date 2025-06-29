#pragma once

#include "IPwm.hpp"

namespace zct {

class PwmReal : public IPwm {
public:
    PwmReal(const char* name, const struct pwm_dt_spec& pwmDtSpec);

    /**
     * @brief Destroy the PWM.
     */
    virtual ~PwmReal() = default;

    /**
     * @brief Set the PWM period and pulse width.
     * 
     * @param periodNs The period of the PWM signal in nanoseconds.
     * @param pulseWidthNs The pulse width of the PWM signal in nanoseconds.
     */
    virtual void set(uint32_t periodNs, uint32_t pulseWidthNs) override;

protected:
    const struct pwm_dt_spec& m_pwmDtSpec;
};

} // namespace zct