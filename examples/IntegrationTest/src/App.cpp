#pragma once

#include "App.hpp"

App::App(IPeripherals& peripherals)
    : m_peripherals(peripherals)
{
}

App::~App()
{
}