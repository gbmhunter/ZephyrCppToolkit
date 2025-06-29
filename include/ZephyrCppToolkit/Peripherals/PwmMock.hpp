#pragma once

#include "IPwm.hpp"

namespace zct {

class PwmMock : public IPwm {
public:
    PwmMock(const char* name);

    /**
     * @brief Destroy the PWM.
     */
    virtual ~PwmMock() = default;

    /**
     * @brief Set the PWM period and pulse width.
     * 
     * @param periodNs The period of the PWM signal in nanoseconds.
     * @param pulseWidthNs The pulse width of the PWM signal in nanoseconds.
     */
    virtual void set(uint32_t periodNs, uint32_t pulseWidthNs) override;

protected:
};

} // namespace zct