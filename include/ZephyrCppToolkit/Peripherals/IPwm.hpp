#pragma once

namespace zct {

class IPwm {
public:
    IPwm(const char* name);

    /**
     * @brief Destroy the PWM.
     */
    virtual ~IPwm() = default;

    /**
     * @brief Set the PWM period and pulse width.
     * 
     * @param periodNs The period of the PWM signal in nanoseconds.
     * @param pulseWidthNs The pulse width of the PWM signal in nanoseconds.
     */
    virtual void set(uint32_t periodNs, uint32_t pulseWidthNs) = 0;
protected:
    const char* m_name;
};

} // namespace zct