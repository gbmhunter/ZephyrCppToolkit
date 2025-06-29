#pragma once

#include "ZephyrCppToolkit/Peripherals/IGpio.hpp"

class IPeripherals {
public:
    virtual ~IPeripherals() = default;

    virtual zct::IGpio& getInputGpio() = 0;
    virtual zct::IGpio& getOutputGpio() = 0;
};