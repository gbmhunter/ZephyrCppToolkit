#pragma once

// System includes
#include <cstdint>

namespace zct {

class IAdc {
public:
    IAdc(const char* name);

    ~IAdc();

    /**
     * Perform a single ADC conversion on the channel and return the result in mV.
     * 
     * \return The result of the ADC conversion in mV.
     */
    virtual int32_t readMv() = 0;

protected:
    const char* m_name;
};

} // namespace zct