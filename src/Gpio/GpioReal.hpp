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
     * Set the physical value of the GPIO.
     * 
     * This ignores the logic mode of the GPIO and sets the physical value directly.
     * 
     * @param value The physical value to set.
     */
    void setPhysical(bool value) override;

    /**
     * Get the physical value of the GPIO.
     * 
     * This ignores the logic mode of the GPIO and returns the physical value directly.
     * 
     * @return The physical value of the GPIO.
     */
    bool getPhysical() const override;

    /**
     * Set the direction of the GPIO.
     * 
     * @param direction The direction to set.
     */
    void setDirection(Direction direction) override;

private:
    const struct gpio_dt_spec* m_spec;
};

} // namespace zct