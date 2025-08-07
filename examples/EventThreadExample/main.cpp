#include <variant>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Events/EventThread.hpp"

LOG_MODULE_REGISTER(EventThreadTests, LOG_LEVEL_DBG);

//================================================================================================//
// EVENTS
//================================================================================================//

namespace Events {

struct Exit {};

struct LedFlashing {
    uint32_t flashRateMs;
};

struct TimerExpired {};

// Create a generic event type that can be anyone of the specific events above.
typedef std::variant<LedFlashing, Exit, TimerExpired> Generic;

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
            7,
            EVENT_QUEUE_NUM_ITEMS
        ),
        m_flashingTimer("FlashingTimer", [this]() {
            Events::TimerExpired timerExpiredEvent;
            handleEvent(timerExpiredEvent);
        })
    {
        // Register timers
        m_eventThread.timerManager().registerTimer(m_flashingTimer);
        
        // Register external event callback
        m_eventThread.onExternalEvent([this](const Events::Generic& event) {
            handleEvent(event);
        });
        
        // Start the event loop (it runs it it's own thread)
        m_eventThread.start();
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
        // This is how use send "external" events (i.e. not timer events) to
        // the event thread. The event thread will call handleEvent() when it
        // receives this event.
        m_eventThread.sendEvent(ledFlashingEvent);
    }

private:
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);
    zct::EventThread<Events::Generic> m_eventThread;
    zct::Timer m_flashingTimer;
    bool m_ledIsOn = false;

    void handleEvent(const Events::Generic& event) {
        if (std::holds_alternative<Events::LedFlashing>(event)) {
            // Start the timer to flash the LED
            m_flashingTimer.start(1000, 1000);
            LOG_INF("Starting flashing. Turning LED on...");
            m_ledIsOn = true;
        } else if (std::holds_alternative<Events::Exit>(event)) {
            // This will cause the event loop to end (and the thread to exit)
            // once we return from handleEvent().
            m_eventThread.exitEventLoop();
        } else if (std::holds_alternative<Events::TimerExpired>(event)) {
            LOG_INF("Toggling LED to %d.", !m_ledIsOn);
            m_ledIsOn = !m_ledIsOn;
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