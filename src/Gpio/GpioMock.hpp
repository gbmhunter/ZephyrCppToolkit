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
     * @copydoc zct::IGpio::set(bool)
     */
    void set(bool value) override;

    /**
     * @copydoc zct::IGpio::get()
     */
    bool get() const override;

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
    bool m_value;
};

} // namespace zct