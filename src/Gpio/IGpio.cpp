#include <zephyr/kernel.h>

#include "ZephyrCppToolkit/Peripherals/IGpio.hpp"

namespace zct {

IGpio::IGpio(const char* name, Direction direction, LogicMode logicMode) 
    : m_name(name), m_direction(direction), m_logicMode(logicMode)
{
    // Make sure the name is not null
    __ASSERT_NO_MSG(name != nullptr);
}

void IGpio::set(bool value) {
    if (m_logicMode == LogicMode::ActiveHigh) {
        setPhysical(value);
    } else if (m_logicMode == LogicMode::ActiveLow) {
        // Invert logic
        setPhysical(!value);
    } else {
        __ASSERT_NO_MSG(false);
    }
    
}

bool IGpio::get() const {
    if (m_logicMode == LogicMode::ActiveHigh) {
        return getPhysical();
    } else if (m_logicMode == LogicMode::ActiveLow) {
        // Invert logic
        return !getPhysical();
    } else {
        __ASSERT_NO_MSG(false);
    }
}

void IGpio::setDirection(Direction direction) {
    m_direction = direction;
    configurePinBasedOnSettings();
}

void IGpio::setLogicMode(LogicMode logicMode) {
    m_logicMode = logicMode;
    configurePinBasedOnSettings();
}

// void IGpio::configureInterrupt(InterruptMode interruptMode, std::function<void()> callback) {
//     // m_interruptMode = interruptMode;
//     // m_interruptCallback = callback;
//     // configurePinBasedOnSettings();
// }

} // namespace zct