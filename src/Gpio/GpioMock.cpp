#include <zephyr/logging/log.h>

#include "GpioMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_GpioMock, LOG_LEVEL_INF);

GpioMock::GpioMock(const char* name) {
    m_name = name;
    m_value = false;
    LOG_DBG("Created GPIO mock %s.", m_name);
}

GpioMock::~GpioMock() {}

void GpioMock::set(bool value) {
    LOG_DBG("Setting GPIO %s to %s.", m_name, value ? "on" : "off");
    m_value = value;
}

bool GpioMock::get() {
    LOG_DBG("Getting GPIO %s. Value: %d.", m_name, m_value);
    return m_value;
}

} // namespace zct