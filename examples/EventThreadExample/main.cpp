#include <variant>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit.h"

namespace zct = zephyr_cpp_toolkit;

LOG_MODULE_REGISTER(EventThreadTests, LOG_LEVEL_DBG);

//================================================================================================//
// EVENTS
//================================================================================================//

struct MyTimerTimeoutEvent {
};

struct ExitEvent {};

struct LedFlashingEvent {
    uint32_t flashRateMs;
};

typedef std::variant<MyTimerTimeoutEvent, LedFlashingEvent, ExitEvent> Event;

//================================================================================================//
// EVENT THREAD
//================================================================================================//

class MyEventThread : public zct::EventThread<Event> {
public:
    MyEventThread() :
        zct::EventThread<Event>(threadStack, THREAD_STACK_SIZE, EVENT_QUEUE_NUM_ITEMS),
        m_flashingTimer()
    {
        // Register timers
        m_timerManager.registerTimer(m_flashingTimer);
    }

private:
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(threadStack, THREAD_STACK_SIZE);
    zct::Timer<Event> m_flashingTimer;
    bool m_ledIsOn = false;

    void threadMain() override {
        while (1) {
            Event event = zct::EventThread<Event>::waitForEvent();
            LOG_DBG("Event received: %d.", event.index());
            if (std::holds_alternative<MyTimerTimeoutEvent>(event)) {
                m_ledIsOn = !m_ledIsOn;
            } else if (std::holds_alternative<LedFlashingEvent>(event)) {
                LOG_DBG("Toggling LED to %d.", m_ledIsOn);
                // Start the timer to flash the LED
                m_flashingTimer.start(1000, 1000);
                LOG_DBG("Starting flashing. Turning LED on...");
                m_ledIsOn = true;
            } else if (std::holds_alternative<ExitEvent>(event)) {
                LOG_DBG("Got ExitEvent.");
                break;
            }
        }
    }
};

int main() {
    MyEventThread eventThread;

    // Send a LED on event
    LedFlashingEvent ledFlashingEvent = { .flashRateMs = 1000 }; // Create an Event lvalue directly
    eventThread.sendEvent(ledFlashingEvent);

    k_sleep(K_MSEC(500));

    // Send an exit event
    ExitEvent exitEvent;
    eventThread.sendEvent(exitEvent);
}