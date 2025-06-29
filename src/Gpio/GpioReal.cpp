#include "GpioReal.hpp"

#include <zephyr/logging/log.h>

namespace zct {

LOG_MODULE_REGISTER(GpioReal, LOG_LEVEL_INF);

GpioReal::GpioReal(const char* name, const struct gpio_dt_spec* spec, Direction direction, LogicMode logicMode) 
    : IGpio(name, direction, logicMode)
{
    m_spec = spec;

    bool isReady = gpio_is_ready_dt(m_spec);
    __ASSERT_NO_MSG(isReady);

    gpio_flags_t flags = 0;

    if (direction == Direction::Output) {
        // Also configure as input so that we can read the value. Without this it always returns 0!
        flags |= GPIO_OUTPUT_INACTIVE | GPIO_INPUT;
    } else if (direction == Direction::Input) {
        flags |= GPIO_INPUT;
    } else {
        __ASSERT_NO_MSG(false);
    }

    if (logicMode == LogicMode::ActiveHigh) {
        flags |= GPIO_ACTIVE_HIGH;
    } else if (logicMode == LogicMode::ActiveLow) {
        flags |= GPIO_ACTIVE_LOW;
    } else {
        __ASSERT_NO_MSG(false);
    }

    int isConfigured = gpio_pin_configure_dt(m_spec, flags);
    __ASSERT_NO_MSG(isConfigured == 0);
}

GpioReal::~GpioReal() {}

void GpioReal::set(bool value) {
    LOG_DBG("Setting GPIO %s to %s.", m_name, value ? "on" : "off");
    gpio_pin_set_dt(m_spec, value ? 1 : 0);
}

bool GpioReal::get() const {
    LOG_DBG("Getting GPIO %s. Value: %d.", m_name, gpio_pin_get_dt(m_spec));
    return gpio_pin_get_dt(m_spec) == 1;
}

} // namespace zct