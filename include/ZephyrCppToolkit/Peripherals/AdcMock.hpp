#pragma once

#include <cstdint>

#include "IAdc.hpp"

namespace zct {

class AdcMock : public IAdc {
public:
    AdcMock(const char* name);
    ~AdcMock();

    /**
     * Perform a single ADC conversion on the channel and return the result in mV.
     * 
     * \return The result of the ADC conversion in mV.
     */
    int32_t readMv() override;

    /**
     * Mock the analogue input voltage to the ADC. Calls to readMv() after this call will return this value.
     * 
     * \param mv The mock ADC reading in mV.
     */
    void mockSetMv(int32_t mv);

protected:
    int32_t m_mockMv = 0;
};

} // namespace zct