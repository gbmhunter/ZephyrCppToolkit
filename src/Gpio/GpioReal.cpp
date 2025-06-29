#include "GpioReal.hpp"

#include <zephyr/logging/log.h>

namespace zct {

LOG_MODULE_REGISTER(GpioReal, LOG_LEVEL_INF);

GpioReal::GpioReal(const char* name, const struct gpio_dt_spec* spec, Direction direction) 
    : IGpio(name, direction)
{
    m_spec = spec;

    bool isReady = gpio_is_ready_dt(m_spec);
    __ASSERT_NO_MSG(isReady);

    // Also configure as input so that we can read the value. Without this it always returns 0!
    int isConfigured = gpio_pin_configure_dt(m_spec, GPIO_OUTPUT_INACTIVE | GPIO_INPUT);
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