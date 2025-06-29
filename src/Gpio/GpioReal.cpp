#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/GpioReal.hpp"

namespace zct {

LOG_MODULE_REGISTER(GpioReal, LOG_LEVEL_DBG);

GpioReal::GpioReal(const char* name, const struct gpio_dt_spec* spec, Direction direction, LogicMode logicMode) 
    : IGpio(name, direction, logicMode)
{
    m_spec = spec;

    bool isReady = gpio_is_ready_dt(m_spec);
    __ASSERT_NO_MSG(isReady);

    configurePinBasedOnSettings();
}

GpioReal::~GpioReal() {}

void GpioReal::setPhysical(bool value) {
    LOG_DBG("Setting GPIO %s to %s.", m_name, value ? "on" : "off");
    int rc = gpio_pin_set_dt(m_spec, value ? 1 : 0);
    __ASSERT_NO_MSG(rc == 0);
    // Get value
    LOG_DBG("gpio_get_dt(%s) = %d", m_spec->port->name, gpio_pin_get_dt(m_spec));
}

bool GpioReal::getPhysical() const {
    LOG_DBG("Getting GPIO %s. Value: %d.", m_name, gpio_pin_get_dt(m_spec));
    return gpio_pin_get_dt(m_spec) == 1;
}

void GpioReal::configurePinBasedOnSettings() {
    gpio_flags_t flags = 0;

    if (m_direction == Direction::Output) {
        flags |= GPIO_OUTPUT_INACTIVE | GPIO_INPUT;
    } else if (m_direction == Direction::Input) {
        flags |= GPIO_INPUT;
    } else {
        __ASSERT_NO_MSG(false);
    }

    if (m_logicMode == LogicMode::ActiveHigh) {
        flags |= GPIO_ACTIVE_HIGH;
    } else if (m_logicMode == LogicMode::ActiveLow) {
        flags |= GPIO_ACTIVE_LOW;
    } else {
        __ASSERT_NO_MSG(false);
    }

    int rc = gpio_pin_configure_dt(m_spec, flags);
    __ASSERT_NO_MSG(rc == 0);
}

void GpioReal::configureInterrupt(InterruptMode interruptMode, std::function<void()> callback) {
    gpio_flags_t flags = 0;

    if (interruptMode == InterruptMode::Disable) {
        flags = GPIO_INT_DISABLE;
    } else if (interruptMode == InterruptMode::EdgeRising) {
        flags = GPIO_INT_EDGE_RISING;
    } else if (interruptMode == InterruptMode::EdgeFalling) {
        flags = GPIO_INT_EDGE_FALLING;
    } else if (interruptMode == InterruptMode::EdgeBoth) {
        flags = GPIO_INT_EDGE_BOTH;
    } else if (interruptMode == InterruptMode::LevelLow) {
        flags = GPIO_INT_LEVEL_LOW;
    } else if (interruptMode == InterruptMode::LevelHigh) {
        flags = GPIO_INT_LEVEL_HIGH;
    } else if (interruptMode == InterruptMode::LevelToInactive) {
        flags = GPIO_INT_EDGE_TO_INACTIVE;
    } else if (interruptMode == InterruptMode::LevelToActive) {
        flags = GPIO_INT_EDGE_TO_ACTIVE;
    } else if (interruptMode == InterruptMode::LevelInactive) {
        flags = GPIO_INT_LEVEL_INACTIVE;
    } else if (interruptMode == InterruptMode::LevelActive) {
        flags = GPIO_INT_LEVEL_ACTIVE;
    } else {
        __ASSERT(false, "Got unsupported interrupt mode. Interrupt mode: %d.", static_cast<int>(interruptMode));
    }
    int rc = gpio_pin_interrupt_configure_dt(m_spec, flags);
    __ASSERT_NO_MSG(rc == 0);

    m_userInterruptCallback = callback;

    // Populate callback data
    m_gpioCallbackDataAndObject.m_obj = this;
    gpio_init_callback(&m_gpioCallbackDataAndObject.m_gpioCallbackData, &interruptCallback, BIT(m_spec->pin));
    gpio_add_callback(m_spec->port, &m_gpioCallbackDataAndObject.m_gpioCallbackData);
}

void GpioReal::interruptCallback(const struct device* dev, struct gpio_callback* cb, gpio_port_pins_t pins)
{
    // WARNING: This will be called in a interrupt context.
    GpioCallbackDataAndObject* gpioCallbackDataAndObject = CONTAINER_OF(cb, GpioCallbackDataAndObject, m_gpioCallbackData);
    GpioReal* obj = gpioCallbackDataAndObject->m_obj;
    // // Now we have the object, call the user provided callback function
    if (obj->m_userInterruptCallback != nullptr) {
        obj->m_userInterruptCallback();
    } else {
        LOG_WRN("User interrupt callback is null.");
    }
}

} // namespace zct