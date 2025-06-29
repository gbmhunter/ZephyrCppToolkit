#include <zephyr/logging/log.h>

#include "GpioMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_GpioMock, LOG_LEVEL_INF);

GpioMock::GpioMock(const char* name, Direction direction) : IGpio(name, direction) {
    m_value = false;
    LOG_DBG("Created GPIO mock %s.", m_name);
}

GpioMock::~GpioMock() {}

void GpioMock::set(bool value) {
    LOG_DBG("Setting GPIO \"%s\" to %s.", m_name, value ? "on" : "off");
    // Make sure only output GPIOs can be set
    __ASSERT_NO_MSG(m_direction == Direction::Output);
    m_value = value;
}

bool GpioMock::get() const {
    LOG_DBG("Getting GPIO \"%s\". Value: %d.", m_name, m_value);
    return m_value;
}

void GpioMock::mockSetInput(bool value) {
    LOG_DBG("Mocking input GPIO \"%s\" to %s.", m_name, value ? "on" : "off");
    m_value = value;
}

} // namespace zct