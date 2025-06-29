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
     * @brief Set the physical value of the GPIO.
     * 
     * This ignores the logic mode of the GPIO and sets the physical value directly.
     * 
     * @param value The physical value to set.
     */
    void setPhysical(bool value) override;

    /**
     * @brief Get the physical value of the GPIO.
     * 
     * This ignores the logic mode of the GPIO and returns the physical value directly.
     * 
     * @return The physical value of the GPIO.
     */
    bool getPhysical() const override;

    /**
     * @brief Configure an interrupt on the GPIO.
     * 
     * The mock implementation of this just stores the interrupt mode and callback into the member variables.
     *
     * Due to mock limitations, when the GPIO is an input the callback will be called correctly for edge based interrupt modes (it will be called when mockSetInput() or mockSetInputPhysical() are called), but not for level based interrupt modes.
     * 
     * @param interruptMode The interrupt mode to set.
     * @param callback The callback to call when the interrupt occurs.
     */
    void configureInterrupt(InterruptMode interruptMode, std::function<void()> callback) override;

    /**
     * @brief Set the logic mode of the GPIO.
     *
     * We override this in the mock because if we change the logic mode we need
     * to preserve the current physical value, not logical. For example, a GPIO
     * input starts of at 0V, inactive. If we change the logic mode to active
     * low, the physical value will still be 0V, but the logical value will be active.
     *
     * This could also cause logical level interrupts to be triggered.
     * 
     * @param logicMode The logic mode to set.
     */
    void setLogicMode(LogicMode logicMode) override;

    /**
     * @brief Use this to pretend to be an external
     * signal changing the state of an input GPIO. This
     * sets the logical value of the GPIO.
     * 
     * Has no effect if the GPIO is configured as an output.
     *
     * @sa mockSetInputPhysical()
     *
     * @param logicalValue The logical value to set the GPIO to.
     */
    void mockSetInput(bool logicalValue);

    /**
     * @brief Use this to pretend to be an external
     * signal changing the state of an input GPIO. This
     * sets the physical value of the GPIO.
     * 
     * Has no effect if the GPIO is configured as an output.
     *
     * @sa mockSetInput()
     *
     * @param physicalValue The physical value to set the GPIO to.
     */
    void mockSetInputPhysical(bool physicalValue);

protected:
    /**
     * @brief Configure the pin based on the current settings.
     * 
     * The real GPIO will call the Zephyr gpio_pin_configure_dt() function.
     * The mock GPIO will do nothing.
     */
    void configurePinBasedOnSettings() override;

    /**
     * @brief Helper function to determine and call the interrupt handler if needed. Calculates if it needs to be called based on the old and new GPIO values.
     * In the real GPIO this would be done by the Zephyr GPIO driver.
     * 
     * @param oldLogicalValue The old logical value of the GPIO.
     * @param oldPhysicalValue The old physical value of the GPIO.
     * @param newLogicalValue The new logical value of the GPIO.
     * @param newPhysicalValue The new physical value of the GPIO.
     */
    void callInterruptHandlerIfNeeded(
        bool oldLogicalValue,
        bool oldPhysicalValue,
        bool newLogicalValue,
        bool newPhysicalValue);

    /**
     * Because it is a mock GPIO, we store the value of the GPIO here.
     */
    bool m_logicalValue;
};

} // namespace zct