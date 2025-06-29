#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "ZephyrCppToolkit/Peripherals/GpioMock.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_GpioMock, LOG_LEVEL_DBG);

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

void GpioMock::configureInterrupt(InterruptMode interruptMode, std::function<void()> callback) {
    LOG_DBG("Configuring interrupt on GPIO \"%s\" in mode %d.", m_name, interruptMode);
    m_interruptMode = interruptMode;
    m_interruptUserCallback = callback;
}

void GpioMock::setLogicMode(LogicMode logicMode) {
    // We need to preserve the current physical value, not logical
    bool oldPhysicalValue = getPhysical();
    m_logicMode = logicMode;
    // Re-apply the old physical value now that the logic mode has changed
    mockSetInputPhysical(oldPhysicalValue);
}

void GpioMock::configurePinBasedOnSettings() {
    // Do nothing
}

void GpioMock::mockSetInput(bool logicalValue) {
    LOG_DBG("Mocking input GPIO \"%s\" to %s.", m_name, logicalValue ? "on" : "off");
    bool oldLogicalValue = m_logicalValue;
    bool oldPhysicalValue = getPhysical();

    // We've got the old values, so safe to set the new value
    m_logicalValue = logicalValue;
    bool newLogicalValue = m_logicalValue; // Just for naming consistency
    bool newPhysicalValue = getPhysical();

    callInterruptHandlerIfNeeded(oldLogicalValue, oldPhysicalValue, newLogicalValue, newPhysicalValue);
}

void GpioMock::mockSetInputPhysical(bool physicalValue) {
    LOG_DBG("Mocking input GPIO \"%s\" to %s.", m_name, physicalValue ? "on" : "off");
    bool oldLogicalValue = m_logicalValue;
    bool oldPhysicalValue = getPhysical();

    // We've got the old values, so safe to set the new value
    if (m_logicMode == LogicMode::ActiveHigh) {
        m_logicalValue = physicalValue;
    } else if (m_logicMode == LogicMode::ActiveLow) {
        m_logicalValue = !physicalValue;
    } else {
        __ASSERT_NO_MSG(false);
    }

    bool newLogicalValue = m_logicalValue; // Just for naming consistency
    bool newPhysicalValue = physicalValue;

    callInterruptHandlerIfNeeded(oldLogicalValue, oldPhysicalValue, newLogicalValue, newPhysicalValue);
}

void GpioMock::callInterruptHandlerIfNeeded(
    bool oldLogicalValue,
    bool oldPhysicalValue,
    bool newLogicalValue,
    bool newPhysicalValue)
{
    LOG_DBG("%s: Checking if we need to call the interrupt handler.", m_name);
    //===============================================
    // Check if we need to call the interrupt handler
    //===============================================
    bool callInterruptHandler = false;
    // Make sure these are checked in the same order as the enum in IGpio.hpp
    // (just to make sure we don't miss any cases)
    if (m_interruptMode == InterruptMode::Disable) {
        callInterruptHandler = false;
    } else if (m_interruptMode == InterruptMode::EdgeRising) {
        callInterruptHandler = oldPhysicalValue == false && newPhysicalValue == true;
    } else if (m_interruptMode == InterruptMode::EdgeFalling) {
        callInterruptHandler = oldPhysicalValue == true && newPhysicalValue == false;
    } else if (m_interruptMode == InterruptMode::EdgeBoth) {
        callInterruptHandler = oldPhysicalValue != newPhysicalValue;
    } else if (m_interruptMode == InterruptMode::LevelLow) {
        callInterruptHandler = newPhysicalValue == false;
    } else if (m_interruptMode == InterruptMode::LevelHigh) {
        callInterruptHandler = newPhysicalValue == true;
    } else if (m_interruptMode == InterruptMode::LevelToInactive) {
        callInterruptHandler = oldLogicalValue == true && newLogicalValue == false;
    } else if (m_interruptMode == InterruptMode::LevelToActive) {
        callInterruptHandler = oldLogicalValue == false && newLogicalValue == true;
    } else if (m_interruptMode == InterruptMode::LevelInactive) {
        callInterruptHandler = newLogicalValue == false;
    } else if (m_interruptMode == InterruptMode::LevelActive) {
        callInterruptHandler = newLogicalValue == true;
    } else {
        __ASSERT(false, "Invalid interrupt mode: %d.", static_cast<int>(m_interruptMode));
    }

    LOG_DBG("%s: Call interrupt handler: %d, m_interruptUserCallback: %p.", m_name, callInterruptHandler, m_interruptUserCallback);
    if (callInterruptHandler && m_interruptUserCallback != nullptr) {
        m_interruptUserCallback();
    }
}

} // namespace zct