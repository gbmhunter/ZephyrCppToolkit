#pragma once

#include "IGpio.hpp"

namespace zct {

/**
 * Implements a mock GPIO pin.
 */
class GpioMock : public IGpio {
public:
    GpioMock(const char* name);
    ~GpioMock();

    /**
     * @copydoc zct::IGpio::set(bool)
     */
    void set(bool value) override;

    /**
     * @inherit
     */
    bool get() override;

private:
    const char* m_name;
    bool m_value;
};

} // namespace zct