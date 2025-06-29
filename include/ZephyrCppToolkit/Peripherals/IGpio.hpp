#pragma once

#include <functional>

namespace zct {

class IGpio {
public:

    enum class Direction {
        Input,
        Output
    };

    /**
     * Enumerates the possible logic modes of a GPIO.
     * - Active high means that true is equal to a high voltage, false is 0V.
     * - Active low means that true is equal to 0V, false is high voltage.
     */
    enum class LogicMode {
        ActiveHigh,
        ActiveLow
    };

    /**
     * Enumerates the possible interrupt modes of a GPIO.
     */
    enum class InterruptMode {
        Disable,
        EdgeRising,
        EdgeFalling,
        EdgeBoth,
        LevelLow,
        LevelHigh,
        LevelToInactive,
        LevelToActive,
        LevelInactive,
        LevelActive
    };

    /**
     * @brief Create a new GPIO.
     * 
     * @param name The name of the GPIO. Used for logging purposes.
     * @param direction The direction of the GPIO.
     * @param logicMode The logic mode of the GPIO.
     */
    IGpio(const char* name, Direction direction = Direction::Input, LogicMode logicMode = LogicMode::ActiveHigh);

    /**
     * @brief Destroy the GPIO.
     */
    virtual ~IGpio() = default;

    /**
     * Set the logical value of the GPIO.
     *
     * Will assert if the GPIO is not configured as an output.
     * 
     * @param value The logical value to set.
     */
    virtual void set(bool value);

    /**
     * Get the logical value of the GPIO.
     * 
     * Can be called on an output or input GPIO.
     * 
     * @return The logical value of the GPIO.
     */
    virtual bool get() const;

    /**
     * Set the physical value of the GPIO.
     * 
     * This ignores the logic mode of the GPIO and sets the physical value directly.
     * 
     * @param value The physical value to set.
     */
    virtual void setPhysical(bool value) = 0;

    /**
     * Get the physical value of the GPIO.
     * 
     * This ignores the logic mode of the GPIO and returns the physical value directly.
     * 
     * @return The physical value of the GPIO.
     */
    virtual bool getPhysical() const = 0;

    /**
     * Set the direction of the GPIO.
     * 
     * @param direction The direction to set.
     */
    virtual void setDirection(Direction direction);

    /**
     * Set the logic mode of the GPIO.
     * 
     * @param logicMode The logic mode to set.
     */
    virtual void setLogicMode(LogicMode logicMode);

    /**
     * Configure an interrupt on the GPIO.
     * 
     * @param interruptMode The interrupt mode to set.
     * @param callback The callback to call when the interrupt occurs.
     */
    virtual void configureInterrupt(InterruptMode interruptMode, std::function<void()> callback) = 0;

protected:
    const char* m_name;
    Direction m_direction;
    LogicMode m_logicMode;

    /**
     * Configure the pin based on the current settings.
     * The real GPIO will call the Zephyr gpio_pin_configure_dt() function.
     * The mock GPIO will do nothing.
     */
    virtual void configurePinBasedOnSettings() = 0;
};

} // namespace zct