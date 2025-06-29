
#pragma once

#include <variant>

#include "ZephyrCppToolkit/EventThreadv2.hpp"
#include "ZephyrCppToolkit/Timer.hpp"

#include "IPeripherals.hpp"

namespace AppEvents {
    struct InputGpioWentActive {};
    struct GpioTimerExpired {};
    struct ExitCmd {};

    using Generic = std::variant<InputGpioWentActive, GpioTimerExpired, ExitCmd>;
}

class App {
public:
    App(IPeripherals& peripherals);
    ~App();

    void threadFunction();

protected:
    IPeripherals& m_peripherals;
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(threadStack, THREAD_STACK_SIZE);
    zct::EventThreadv2<AppEvents::Generic> m_eventThread;

    zct::Timer<AppEvents::Generic> m_gpioTimer;
};