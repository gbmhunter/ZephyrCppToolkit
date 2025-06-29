#pragma once

#include <zephyr/drivers/gpio.h>

#include "IGpio.hpp"

namespace zct {

/**
 * Implements a real Zephyr GPIO pin.
 */
class GpioReal : public IGpio {
public:

    /**
     * @brief Create a new real GPIO pin.
     * 
     * @param name The name of the GPIO. Used for logging purposes.
     * @param spec The Zephyr GPIO DT spec struct.
     * @param direction The direction of the GPIO.
     */
    GpioReal(const char* name, const struct gpio_dt_spec* spec, Direction direction = Direction::Input, LogicMode logicMode = LogicMode::ActiveHigh);

    /**
     * @brief Destroy the GPIO. Does nothing.
     */
    ~GpioReal();

    /**
     * @copydoc zct::IGpio::set(bool)
     */
    void set(bool value) override;
    bool get() const override;

private:
    const struct gpio_dt_spec* m_spec;
};

} // namespace zct