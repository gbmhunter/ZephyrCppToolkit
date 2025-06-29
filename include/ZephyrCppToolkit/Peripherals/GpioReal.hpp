#pragma once

#include <functional>

#include <zephyr/drivers/gpio.h>

#include "IGpio.hpp"

namespace zct {

// Forward declaration of the GpioReal class.
class GpioReal;

/**
 * This exists so that we don't get the compiler warning:
 * warning: 'offsetof' within non-standard-layout type 'MyClass' is conditionally-supported [-Winvalid-offsetof]
 * 
 * This makes sure the containing object for the gpio_callback has a standard layout.
 */
struct GpioCallbackDataAndObject {
    GpioReal* m_obj;
    struct gpio_callback m_gpioCallbackData;
};

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
     * Configure an interrupt on the GPIO.
     * 
     * @param interruptMode The interrupt mode to set.
     * @param callback The callback to call when the interrupt occurs. This will be called in a interrupt context.
     */
    void configureInterrupt(InterruptMode interruptMode, std::function<void()> callback) override;

protected:
    const struct gpio_dt_spec* m_spec;

    /**
     * Structure for holding callback info, if used.
     * 
     * Needs to be a pointer to a member of the class.
     */
    // struct gpio_callback m_gpioCallbackData;

    GpioCallbackDataAndObject m_gpioCallbackDataAndObject;

    std::function<void()> m_userInterruptCallback;

    /**
     * Configure the pin based on the current settings.
     * The real GPIO will call the Zephyr gpio_pin_configure_dt() function.
     * The mock GPIO will do nothing.
     */
    void configurePinBasedOnSettings() override;

    /**
     * Static callback handler function which has the correct signature so that it can be passed to Zephyr's
     * gpio_init_callback() function.
     * 
     * @param dev The device that triggered the interrupt.
     * @param cb The callback structure.
     * @param pins The pins that triggered the interrupt.
     */
    static void interruptCallback(const struct device* dev, struct gpio_callback* cb, gpio_port_pins_t pins);


};

} // namespace zct