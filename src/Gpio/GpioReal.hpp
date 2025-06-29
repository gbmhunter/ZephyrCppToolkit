#pragma once

#include <zephyr/drivers/gpio.h>

#include "IGpio.hpp"

namespace zct {

/**
 * Implements a real Zephyr GPIO pin.
 */
class GpioReal : public IGpio {
public:
    GpioReal(const char* name, const struct gpio_dt_spec* spec);
    ~GpioReal();

    void set(bool value) override;
    bool get() override;

private:
    const char* m_name;
    const struct gpio_dt_spec* m_spec;
};

} // namespace zct