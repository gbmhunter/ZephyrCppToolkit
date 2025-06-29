#include <variant>

#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "ZephyrCppToolkit.h"

namespace zct = zephyr_cpp_toolkit;

LOG_MODULE_REGISTER(EventThreadTests, LOG_LEVEL_DBG);

ZTEST_SUITE(EventThreadTests, NULL, NULL, NULL, NULL, NULL);

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

class Led : public zct::EventThread<Event> {
    public:
        Led() :
            zct::EventThread<Event>(threadStack, THREAD_STACK_SIZE, EVENT_QUEUE_NUM_ITEMS),
            m_flashingTimer()
        {
            // Register timers
            m_timerManager.registerTimer(m_flashingTimer);
        }

        bool getLedIsOn() {
            int mutexRc = 0;
            zct::MutexLockGuard lockGuard = zct::MutexLockGuard(m_ledIsOnMutex, K_FOREVER, mutexRc);
            __ASSERT_NO_MSG(mutexRc == 0);
            return m_ledIsOn;
        }

        void setLedIsOn(bool ledIsOn) {
            int mutexRc = 0;
            zct::MutexLockGuard lockGuard = zct::MutexLockGuard(m_ledIsOnMutex, K_FOREVER, mutexRc);
            __ASSERT_NO_MSG(mutexRc == 0);
            m_ledIsOn = ledIsOn;
        }


    private:
        static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
        static constexpr size_t THREAD_STACK_SIZE = 512;
        K_KERNEL_STACK_MEMBER(threadStack, THREAD_STACK_SIZE);
        zct::Timer<Event> m_flashingTimer;
        bool m_ledIsOn = false;
        zct::Mutex m_ledIsOnMutex;

        void threadMain() override {
            while (1) {
                Event event = zct::EventThread<Event>::waitForEvent();
                LOG_DBG("Event received: %d.", event.index());
                if (std::holds_alternative<MyTimerTimeoutEvent>(event)) {
                    bool ledIsOn = getLedIsOn();
                    LOG_DBG("Got MyTimerTimeoutEvent. ledIsOn currently: %d. Setting to %d.", ledIsOn, !ledIsOn);
                    setLedIsOn(!ledIsOn);
                } else if (std::holds_alternative<LedFlashingEvent>(event)) {
                    LOG_DBG("Got LedFlashingEvent.");
                    // Start the timer to flash the LED
                    m_flashingTimer.start(1000, 1000);
                    LOG_DBG("Got LedFlashingEvent. Starting flashing...");
                    setLedIsOn(true);
                } else if (std::holds_alternative<ExitEvent>(event)) {
                    LOG_DBG("Got ExitEvent.");
                    break;
                }
            }
        }
};

ZTEST(EventThreadTests, testEventThreadCreate)
{
    Led eventThread;
    zassert_true(true, "Event thread created");

    // Send a LED on event
    LedFlashingEvent ledFlashingEvent = { .flashRateMs = 1000 }; // Create an Event lvalue directly
    eventThread.sendEvent(ledFlashingEvent);

    k_sleep(K_MSEC(500));

    // Time is now 0.5s. Led should be on
    LOG_DBG("Checking that LED is on...");
    zassert_true(eventThread.getLedIsOn(), "Led should be on");
    LOG_DBG("Check finished.");

    k_sleep(K_MSEC(1000));

    // Time is now 1.5s. Led should be off
    LOG_DBG("Checking that LED is off...");
    zassert_false(eventThread.getLedIsOn(), "Led should be off");
    LOG_DBG("Check finished.");

    // Send an exit event
    ExitEvent exitEvent;
    eventThread.sendEvent(exitEvent);
}