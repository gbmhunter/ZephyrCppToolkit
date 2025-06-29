
#pragma once

#include <variant>

#include "ZephyrCppToolkit/EventThreadv2.hpp"

#include "IPeripherals.hpp"

namespace AppEvents {
    struct Basic {

    };

    using Generic = std::variant<Basic>;
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
};