#pragma once

#include "IPwm.hpp"

namespace zct {

class PwmReal : public IPwm {
public:
    PwmReal(const char* name);

    /**
     * @brief Destroy the PWM.
     */
    virtual ~PwmReal() = default;
};

} // namespace zct