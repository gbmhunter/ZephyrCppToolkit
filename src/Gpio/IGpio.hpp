#pragma once

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
    virtual void set(bool value) = 0;

    /**
     * Get the logical value of the GPIO.
     * 
     * Can be called on an output or input GPIO.
     * 
     * @return The logical value of the GPIO.
     */
    virtual bool get() const = 0;

protected:
    const char* m_name;
    Direction m_direction;
    LogicMode m_logicMode;
};

} // namespace zct