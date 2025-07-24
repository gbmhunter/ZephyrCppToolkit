// 3rd party includes
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

// Local includes
#include "ZephyrCppToolkit/Peripherals/AdcMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_AdcMock, LOG_LEVEL_WRN);

AdcMock::AdcMock(const char* name) : IAdc(name) {
    LOG_DBG("Created ADC mock %s.", m_name);
}

AdcMock::~AdcMock() {}

int32_t AdcMock::readMv() {
    return m_mockMv;
}

void AdcMock::mockSetMv(int32_t mv) {
    m_mockMv = mv;
}

} // namespace zct