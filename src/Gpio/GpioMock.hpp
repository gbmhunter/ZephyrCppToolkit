#pragma once

#include "IGpio.hpp"

namespace zct {

/**
 * Implements a mock GPIO pin.
 */
class GpioMock : public IGpio {
public:

    /**
     * @brief Create a new mock GPIO pin. Default direction is input.
     * 
     * @param name The name of the GPIO. Used for logging purposes.
     * @param direction The direction of the GPIO.
     */
    GpioMock(const char* name, Direction direction = Direction::Input);

    /**
     * @brief Destroy the GPIO. Does nothing.
     */
    ~GpioMock();

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

    /**
     * @brief Use this to pretend to be an external
     * signal changing the state of an input GPIO.
     * 
     * Has no effect if the GPIO is configured as an output.
     *
     * @param value The value to set the GPIO to.
     */
    void mockSetInput(bool value);

private:
    const char* m_name;

    /**
     * Because it is a mock GPIO, we store the value of the GPIO here.
     */
    bool m_logicalValue;
};

} // namespace zct