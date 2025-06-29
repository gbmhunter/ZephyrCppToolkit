#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/GpioMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_GpioMock, LOG_LEVEL_INF);

GpioMock::GpioMock(const char* name, Direction direction) : IGpio(name, direction) {
    m_logicalValue = false;
    LOG_DBG("Created GPIO mock %s.", m_name);
}

GpioMock::~GpioMock() {}

void GpioMock::setPhysical(bool value) {
    LOG_DBG("Setting GPIO \"%s\" to %s.", m_name, value ? "on" : "off");
    // Make sure only output GPIOs can be set
    __ASSERT_NO_MSG(m_direction == Direction::Output);
    if (m_logicMode == LogicMode::ActiveHigh) {
        m_logicalValue = value;
    } else if (m_logicMode == LogicMode::ActiveLow) {
        m_logicalValue = !value;
    } else {
        __ASSERT_NO_MSG(false);
    }
}

bool GpioMock::getPhysical() const {
    LOG_DBG("Getting GPIO \"%s\". Value: %d.", m_name, m_logicalValue);
    if (m_logicMode == LogicMode::ActiveHigh) {
        return m_logicalValue;
    } else if (m_logicMode == LogicMode::ActiveLow) {
        return !m_logicalValue;
    } else {
        __ASSERT_NO_MSG(false);
    }
}

void GpioMock::configurePinBasedOnSettings() {
    // Do nothing
}

void GpioMock::mockSetInput(bool value) {
    LOG_DBG("Mocking input GPIO \"%s\" to %s.", m_name, value ? "on" : "off");
    m_logicalValue = value;
}

} // namespace zct