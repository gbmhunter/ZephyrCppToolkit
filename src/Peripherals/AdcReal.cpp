#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/AdcReal.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_AdcReal, LOG_LEVEL_WRN);

AdcReal::AdcReal(const char* name, const struct adc_dt_spec * adcSpec) : IAdc(name)
{
    __ASSERT_NO_MSG(adcSpec);
    m_adcSpec = adcSpec;

    bool isReady = adc_is_ready_dt(m_adcSpec);
    __ASSERT(isReady, "ADC \"%s\" is not ready.", m_name);

    int rc = adc_channel_setup_dt(m_adcSpec);
    __ASSERT(rc == 0, "Could not setup ADC channel %s. Got adc_channel_setup_dt() return code %d.", m_name, rc);
}

AdcReal::~AdcReal() {}

int32_t AdcReal::readMv()
{
    int16_t buf = 0;
    struct adc_sequence sequence = {
        .buffer = &buf,
        .buffer_size = sizeof(buf), // Buffer size in bytes, not number of samples
    };

    int rc = adc_sequence_init_dt(m_adcSpec, &sequence);
    __ASSERT(rc == 0, "Could not initalize sequnce");

    rc = adc_read(m_adcSpec->dev, &sequence);
    __ASSERT(rc == 0, "Could not read (%d)", rc);

    int32_t val_mv = buf; // Save the raw value to val_mv, as required by adc_raw_to_millivolts_dt()
    rc = adc_raw_to_millivolts_dt(m_adcSpec, &val_mv);
    // conversion to mV may not be supported, skip if not
    __ASSERT(rc == 0, "Could not convert raw value to mV. adc_raw_to_millivolts_dt() return code %d.", rc);
    return val_mv;
}

} // namespace zct