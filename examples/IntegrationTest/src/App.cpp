#include "ZephyrCppToolkit/EventThreadv2.hpp"

#include "App.hpp"

App::App(IPeripherals& peripherals)
    : 
    m_peripherals(peripherals),
    m_eventThread("App", threadStack, THREAD_STACK_SIZE, EVENT_QUEUE_NUM_ITEMS, [this]() {
        threadFunction();
    })
{
}

App::~App()
{
}

void App::threadFunction()
{
    while (true) {
        auto event = m_eventThread.waitForEvent();
        // std::visit([this](auto&& event) {
    }
}