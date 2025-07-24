#pragma once

#include "IAdc.hpp"

namespace zct {

class AdcReal : public IAdc {
public:
    AdcReal(const char* name, const struct adc_dt_spec * adcSpec);
    ~AdcReal();

    /**
     * Perform a single ADC conversion on the channel and return the result in mV.
     * 
     * \return The result of the ADC conversion in mV.
     */
    int32_t readMv() override;

protected:
    const struct adc_dt_spec * m_adcSpec;
};

} // namespace zct