#pragma once

namespace zct {

class IGpio {
public:
    // GpioBase(); // Constructor not typically needed for pure interface
    virtual ~IGpio() = default; // Ensure virtual destructor

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
    virtual bool get() = 0;
};

} // namespace zct