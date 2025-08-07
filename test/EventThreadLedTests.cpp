#include <variant>

#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "ZephyrCppToolkit/Events/EventThread.hpp"
#include "ZephyrCppToolkit/Events/Timer.hpp"
#include "ZephyrCppToolkit/Core/Mutex.hpp"

LOG_MODULE_REGISTER(EventThreadLedTests, LOG_LEVEL_DBG);

ZTEST_SUITE(EventThreadLedTests, NULL, NULL, NULL, NULL, NULL);

//================================================================================================//
// EVENTS
//================================================================================================//

/** Wrap events in a namespace to avoid name collisions with other events. */
namespace MyEvents {
    struct TimerExpiredEvent {};

    struct ExitEvent {};

    struct LedFlashingEvent {
        uint32_t flashRateMs;
    };

    using Generic = std::variant<TimerExpiredEvent, LedFlashingEvent, ExitEvent>;
} // namespace MyEvents

class Led : public zct::EventThread<MyEvents::Generic> {
    public:
        Led() :
            zct::EventThread<MyEvents::Generic>(
                "Led",
                m_threadStack,
                THREAD_STACK_SIZE,
                [this]() { threadMain(); },
                7,
                EVENT_QUEUE_NUM_ITEMS
            ),
            m_flashingTimer("FlashingTimer", [this]() {
                LOG_DBG("Timer callback called, creating TimerExpiredEvent.");
                MyEvents::TimerExpiredEvent timerEvent;
                handleEvent(timerEvent);
             })
        {
            // Register timers
            m_timerManager.registerTimer(m_flashingTimer);
        }

        bool getLedIsOn() {
            zct::MutexLockGuard lockGuard = m_ledIsOnMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            return m_ledIsOn;
        }

        void setLedIsOn(bool ledIsOn) {
            zct::MutexLockGuard lockGuard = m_ledIsOnMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            m_ledIsOn = ledIsOn;
        }


    private:
        static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
        static constexpr size_t THREAD_STACK_SIZE = 512;
        K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);
        zct::Timer m_flashingTimer;
        bool m_ledIsOn = false;
        zct::Mutex m_ledIsOnMutex;

        void handleEvent(const MyEvents::Generic& event) {
            LOG_DBG("Event received: %d.", event.index());
            if (std::holds_alternative<MyEvents::TimerExpiredEvent>(event)) {
                bool ledIsOn = getLedIsOn();
                LOG_DBG("Got TimerExpiredEvent: ledIsOn currently: %d. Setting to %d.", ledIsOn, !ledIsOn);
                setLedIsOn(!ledIsOn);
            } else if (std::holds_alternative<MyEvents::LedFlashingEvent>(event)) {
                LOG_DBG("Got LedFlashingEvent.");
                // Start the timer to flash the LED
                m_flashingTimer.start(1000, 1000);
                LOG_DBG("Got LedFlashingEvent. Starting flashing...");
                setLedIsOn(true);
            } else if (std::holds_alternative<MyEvents::ExitEvent>(event)) {
                LOG_DBG("Got ExitEvent.");
                // Exit the event loop
                exitEventLoop();
            }
        }

        void threadMain() {
            // Register external event callback
            onExternalEvent([this](const MyEvents::Generic& event) {
                handleEvent(event);
            });

            // Start the event loop (this never returns)
            runEventLoop();
        }
};

ZTEST(EventThreadLedTests, testEventThreadCreate)
{
    Led eventThread;
    zassert_true(true, "Event thread created");

    // Send a LED on event
    MyEvents::LedFlashingEvent ledFlashingEvent = { .flashRateMs = 1000 }; // Create an Event lvalue directly
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
    MyEvents::ExitEvent exitEvent;
    eventThread.sendEvent(exitEvent);
}