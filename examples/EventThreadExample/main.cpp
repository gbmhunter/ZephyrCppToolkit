#include <variant>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Events/EventThread.hpp"

LOG_MODULE_REGISTER(EventThreadTests, LOG_LEVEL_DBG);

//================================================================================================//
// EVENTS
//================================================================================================//

namespace Events {

struct MyTimerExpiry {};

struct Exit {};

struct LedFlashing {
    uint32_t flashRateMs;
};

// Create a generic event type that can be anyone of the specific events above.
typedef std::variant<MyTimerExpiry, LedFlashing, Exit> Generic;

}

//================================================================================================//
// EVENT THREAD
//================================================================================================//

class Led {
public:
    Led() :
        m_eventThread(
            "Led",
            m_threadStack,
            THREAD_STACK_SIZE,
            [this]() { threadMain(); },
            7,
            EVENT_QUEUE_NUM_ITEMS
        ),
        m_flashingTimer("FlashingTimer", Events::MyTimerExpiry())
    {
        // Register timers
        m_eventThread.timerManager().registerTimer(m_flashingTimer);
    }

    ~Led() {
        // Send the exit event to the event thread
        Events::Exit exitEvent;
        m_eventThread.sendEvent(exitEvent);
    }

    /**
     * Start flashing the LED at the given rate.
     * 
     * THREAD SAFE.
     * 
     * @param flashRateMs The rate at which to flash the LED.
     */
    void flash(uint32_t flashRateMs) {
        Events::LedFlashing ledFlashingEvent = { .flashRateMs = flashRateMs };
        m_eventThread.sendEvent(ledFlashingEvent);
    }

private:
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);
    zct::EventThread<Events::Generic> m_eventThread;
    zct::Timer<Events::Generic> m_flashingTimer;
    bool m_ledIsOn = false;

    void threadMain() {
        while (1) {
            Events::Generic event = m_eventThread.waitForEvent();
            if (std::holds_alternative<Events::MyTimerExpiry>(event)) {
                LOG_INF("Toggling LED to %d.", !m_ledIsOn);
                m_ledIsOn = !m_ledIsOn;
            } else if (std::holds_alternative<Events::LedFlashing>(event)) {
                // Start the timer to flash the LED
                m_flashingTimer.start(1000, 1000);
                LOG_INF("Starting flashing. Turning LED on...");
                m_ledIsOn = true;
            } else if (std::holds_alternative<Events::Exit>(event)) {
                break;
            }
        }
    }
};

int main() {
    Led led;

    // Start the LED flashing. The flashing will happen
    // in the LED event thread.
    led.flash(1000);

    // Wait 2.5s. The LED should flash twice in this time.
    k_sleep(K_MSEC(2500));
}