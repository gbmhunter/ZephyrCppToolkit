#include <variant>

#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/IGpio.hpp"
#include "ZephyrCppToolkit/EventThreadv2.hpp"

#include "App.hpp"

LOG_MODULE_REGISTER(App, LOG_LEVEL_DBG);

App::App(IPeripherals& peripherals)
    : 
    m_peripherals(peripherals),
    m_eventThread("App", threadStack, THREAD_STACK_SIZE, EVENT_QUEUE_NUM_ITEMS, [this]() {
        threadFunction();
    }),
    m_gpioTimer(AppEvents::GpioTimerExpired())
{
    m_eventThread.timerManager().registerTimer(m_gpioTimer);
}

App::~App()
{
    m_eventThread.sendEvent(AppEvents::ExitCmd());
}

void App::threadFunction()
{

    // Configure output GPIO
    zct::IGpio& outputGpio = m_peripherals.getOutputGpio();
    outputGpio.setDirection(zct::IGpio::Direction::Output);

    // Grab the input GPIO
    zct::IGpio& inputGpio = m_peripherals.getInputGpio();
    inputGpio.configureInterrupt(zct::IGpio::InterruptMode::LevelToActive, [this]() {
        LOG_WRN("GPIO interrupt called.");
        // WARNING: Called from interrupt context!
        AppEvents::InputGpioWentActive event;
        m_eventThread.sendEvent(event);
    });

    while (true) {
        auto event = m_eventThread.waitForEvent();
        if (std::holds_alternative<AppEvents::InputGpioWentActive>(event)) {
            LOG_INF("Input GPIO went active");

            // Set output GPIO high.
            outputGpio.set(true);

            // Start timer to make GPIO inactive after 1 minute
            m_gpioTimer.start(1000*60, -1);
        } else if (std::holds_alternative<AppEvents::GpioTimerExpired>(event)) {
            LOG_INF("GPIO timer expired");

            // Set output GPIO low.
            outputGpio.set(false);
        } else if (std::holds_alternative<AppEvents::ExitCmd>(event)) {
            LOG_INF("Exit command received");
            break;
        } 
    }
}