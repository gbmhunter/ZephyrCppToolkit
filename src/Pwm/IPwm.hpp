#pragma once

namespace zct {

class IPwm {
public:
    IPwm(const char* name);

    /**
     * @brief Destroy the PWM.
     */
    virtual ~IPwm() = default;

protected:
    const char* m_name;
};

} // namespace zct