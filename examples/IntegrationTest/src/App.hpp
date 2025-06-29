#pragma once

#include "IPeripherals.hpp"

class App {
public:
    App(IPeripherals& peripherals);
    ~App();

protected:
    IPeripherals& m_peripherals;
};