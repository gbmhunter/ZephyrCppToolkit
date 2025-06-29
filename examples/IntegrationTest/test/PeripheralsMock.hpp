#pragma once

#include "IPeripherals.hpp"
#include "ZephyrCppToolkit/Peripherals/GpioMock.hpp"

class PeripheralsMock : public IPeripherals {
public:
    PeripheralsMock()
        : m_inputGpio("GPIO-INPUT")
        , m_outputGpio("GPIO-OUTPUT")
    {
    }

    virtual ~PeripheralsMock() = default;

    virtual zct::IGpio& getInputGpio() override { return m_inputGpio; }
    virtual zct::IGpio& getOutputGpio() override { return m_outputGpio; }

protected:
    zct::GpioMock m_inputGpio;
    zct::GpioMock m_outputGpio;
};