#include <variant>

#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "ZephyrCppToolkit/Events/EventThread.hpp"
#include "ZephyrCppToolkit/Events/Timer.hpp"
#include "ZephyrCppToolkit/Core/Mutex.hpp"
#include "ZephyrCppToolkit/Core/Util.hpp"

namespace {

LOG_MODULE_REGISTER(EventThreadMultipleTimersTests, LOG_LEVEL_DBG);

ZTEST_SUITE(EventThreadMultipleTimersTests, NULL, NULL, NULL, NULL, NULL);

//================================================================================================//
// EVENTS
//================================================================================================//

/** Wrap events in a namespace to avoid name collisions with other events. */
namespace MyEvents {
    struct ExitEvent {};

    struct Timer1ExpiredEvent {};
    struct Timer2ExpiredEvent {};
    struct Timer3ExpiredEvent {};

    using Generic = std::variant<ExitEvent, Timer1ExpiredEvent, Timer2ExpiredEvent, Timer3ExpiredEvent>;
} // namespace MyEvents

class TestClass {
    public:
        TestClass() :
            m_eventThread(
                "TestClass",
                m_threadStack,
                THREAD_STACK_SIZE,
                7,
                EVENT_QUEUE_NUM_ITEMS
            ),
            m_timer1("Timer1", [this]() {
                MyEvents::Timer1ExpiredEvent timerEvent;
                handleEvent(timerEvent);
            }),
            m_timer2("Timer2", [this]() {
                MyEvents::Timer2ExpiredEvent timerEvent;
                handleEvent(timerEvent);
            }),
            m_timer3("Timer3", [this]() {
                MyEvents::Timer3ExpiredEvent timerEvent;
                handleEvent(timerEvent);
            })
        {
            // Register timers
            m_eventThread.timerManager().registerTimer(m_timer1);
            m_eventThread.timerManager().registerTimer(m_timer2);
            m_eventThread.timerManager().registerTimer(m_timer3);
            
            // Register external event callback
            m_eventThread.onExternalEvent([this](const MyEvents::Generic& event) {
                handleEvent(event);
            });
            
            // Start the timers and the event thread
            m_timer1.start(1000, 1000);
            m_timer2.start(2000, 2000);
            m_timer3.start(3000, 3000);
            
            m_eventThread.start();
        }

        uint32_t getTimer1Count() {
            zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            return m_timer1Count;
        }

        uint32_t getTimer2Count() {
            zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            return m_timer2Count;
        }

        uint32_t getTimer3Count() {
            zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            return m_timer3Count;
        }

        zct::EventThread<MyEvents::Generic> m_eventThread;
    private:
        static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
        static constexpr size_t THREAD_STACK_SIZE = 512;
        K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);
        zct::Timer m_timer1;
        zct::Timer m_timer2;
        zct::Timer m_timer3;
        
        
        uint32_t m_timer1Count = 0;
        uint32_t m_timer2Count = 0;
        uint32_t m_timer3Count = 0;

        zct::Mutex m_accessMutex;

        void handleEvent(const MyEvents::Generic& event) {
            LOG_DBG("Event received: %d.", event.index());
            if (std::holds_alternative<MyEvents::ExitEvent>(event)) {
                LOG_DBG("Got ExitEvent.");
                // Exit the event loop
                m_eventThread.exitEventLoop();
            } else if (std::holds_alternative<MyEvents::Timer1ExpiredEvent>(event)) {
                LOG_DBG("Got Timer1ExpiredEvent.");
                zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
                __ASSERT_NO_MSG(lockGuard.didGetLock());
                m_timer1Count++;
                LOG_DBG("Timer1 callback. Count: %d.", m_timer1Count);
            } else if (std::holds_alternative<MyEvents::Timer2ExpiredEvent>(event)) {
                LOG_DBG("Got Timer2ExpiredEvent.");
                zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
                __ASSERT_NO_MSG(lockGuard.didGetLock());
                m_timer2Count++;
                LOG_DBG("Timer2 callback. Count: %d.", m_timer2Count);
            } else if (std::holds_alternative<MyEvents::Timer3ExpiredEvent>(event)) {
                LOG_DBG("Got Timer3ExpiredEvent.");
                zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
                __ASSERT_NO_MSG(lockGuard.didGetLock());
                m_timer3Count++;
                LOG_DBG("Timer3 callback. Count: %d.", m_timer3Count);
            }
        }
};

ZTEST(EventThreadMultipleTimersTests, testEventThreadCreate)
{
    TestClass testClass;

    int64_t startTimeMs = k_uptime_get();

    // All timer counts should be 0
    zassert_equal(testClass.getTimer1Count(), 0, "Timer1 count should be 0. count: %d.", testClass.getTimer1Count());
    zassert_equal(testClass.getTimer2Count(), 0, "Timer2 count should be 0. count: %d.", testClass.getTimer2Count());
    zassert_equal(testClass.getTimer3Count(), 0, "Timer3 count should be 0. count: %d.", testClass.getTimer3Count());

    // Wait for t=1.5s
    zct::Util::sleepUntilSystemTime(startTimeMs + 1500);

    // Time is now 1.5s. Timer 1 should have fired, but not Timer 2 or 3
    LOG_DBG("Checking that Timer1 has fired...");
    zassert_equal(testClass.getTimer1Count(), 1, "Timer1 count should be 1. count: %d.", testClass.getTimer1Count());
    zassert_equal(testClass.getTimer2Count(), 0, "Timer2 count should be 0. count: %d.", testClass.getTimer2Count());
    zassert_equal(testClass.getTimer3Count(), 0, "Timer3 count should be 0. count: %d.", testClass.getTimer3Count());

    // Wait for t=2.5s
    zct::Util::sleepUntilSystemTime(startTimeMs + 2500);

    // Time is now 2.5s. Timer 1 should have fired again, and Timer 2 should have fired once, but not Timer 3.
    zassert_equal(testClass.getTimer1Count(), 2, "Timer1 count should be 2. count: %d.", testClass.getTimer1Count());
    zassert_equal(testClass.getTimer2Count(), 1, "Timer2 count should be 1. count: %d.", testClass.getTimer2Count());
    zassert_equal(testClass.getTimer3Count(), 0, "Timer3 count should be 0. count: %d.", testClass.getTimer3Count());

    // Wait for t=3.5s
    zct::Util::sleepUntilSystemTime(startTimeMs + 3500);

    // Time is now 3.5s. Timer 1 should have fired again, and Timer 2 should have fired again, but not Timer 3.
    zassert_equal(testClass.getTimer1Count(), 3, "Timer1 count should be 3. count: %d.", testClass.getTimer1Count());
    zassert_equal(testClass.getTimer2Count(), 1, "Timer2 count should be 2. count: %d.", testClass.getTimer2Count());
    zassert_equal(testClass.getTimer3Count(), 1, "Timer3 count should be 0. count: %d.", testClass.getTimer3Count());

    // Wait for t=10.5s
    zct::Util::sleepUntilSystemTime(startTimeMs + 10500);

    zassert_equal(testClass.getTimer1Count(), 10, "Timer1 count should be 10. count: %d.", testClass.getTimer1Count());
    zassert_equal(testClass.getTimer2Count(), 5, "Timer2 count should be 5. count: %d.", testClass.getTimer2Count());
    zassert_equal(testClass.getTimer3Count(), 3, "Timer3 count should be 3. count: %d.", testClass.getTimer3Count());

    // Send exit event
    MyEvents::ExitEvent exitEvent;
    testClass.m_eventThread.sendEvent(exitEvent);
}

ZTEST(EventThreadMultipleTimersTests, testRunInLoopWorks)
{
    TestClass testClass;

    // Create atomic variable to count how many times our function has been called. We need an atomic variable
    // as runInLoop() will call the function in the context of the event thread, which is different to the thread
    // we are in now.
    atomic_t count = 0;
    testClass.m_eventThread.runInLoop([&count]() {
        atomic_inc(&count);
    });

    k_sleep(K_MSEC(100));

    // Check the count
    zassert_equal(atomic_get(&count), 1, "Count should be 1. count: %d.", atomic_get(&count));

    // Send exit event
    MyEvents::ExitEvent exitEvent;
    testClass.m_eventThread.sendEvent(exitEvent);
}

} // namespace