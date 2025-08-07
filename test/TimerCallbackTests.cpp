#include <variant>

#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "ZephyrCppToolkit/Events/EventThread.hpp"
#include "ZephyrCppToolkit/Events/Timer.hpp"
#include "ZephyrCppToolkit/Core/Mutex.hpp"
#include "ZephyrCppToolkit/Core/Util.hpp"

namespace {

LOG_MODULE_REGISTER(TimerCallbackTests, LOG_LEVEL_DBG);

ZTEST_SUITE(TimerCallbackTests, NULL, NULL, NULL, NULL, NULL);

namespace MyEvents {
    struct ExitEvent {};
    using Generic = std::variant<ExitEvent>;
} // namespace MyEvents

class CallbackTestClass {
public:
    CallbackTestClass() :
        m_eventThread(
            "CallbackTest",
            m_threadStack,
            THREAD_STACK_SIZE,
            [this]() { threadMain(); },
            7,
            EVENT_QUEUE_NUM_ITEMS
        ),
        m_timer("CallbackTimer", [this]() { onTimerCallback(); })
    {
        m_eventThread.timerManager().registerTimer(m_timer);
    }

    void startTimer() {
        m_timer.start(100, -1); // One-shot timer, 100ms
    }

    void exit() {
        m_eventThread.sendEvent(MyEvents::ExitEvent());
    }

    uint32_t getCallbackCount() const {
        zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
        __ASSERT_NO_MSG(lockGuard.didGetLock());
        return m_callbackCount;
    }

    bool hasExited() const {
        zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
        __ASSERT_NO_MSG(lockGuard.didGetLock());
        return m_hasExited;
    }

    zct::EventThread<MyEvents::Generic> m_eventThread;

private:
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);

    zct::Timer m_timer;
    mutable zct::Mutex m_accessMutex;
    uint32_t m_callbackCount = 0;
    bool m_hasExited = false;

    void onTimerCallback() {
        LOG_DBG("Timer callback called!");
        zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
        __ASSERT_NO_MSG(lockGuard.didGetLock());
        m_callbackCount++;
    }

    void handleEvent(const MyEvents::Generic& event) {
        LOG_DBG("Event received: %d.", event.index());

        if (std::holds_alternative<MyEvents::ExitEvent>(event)) {
            zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            m_hasExited = true;
            LOG_DBG("Exit event received, breaking loop.");
            m_eventThread.exitEventLoop();
        }
    }

    void threadMain() {
        // Register external event callback
        m_eventThread.onExternalEvent([this](const MyEvents::Generic& event) {
            handleEvent(event);
        });

        // Start the event loop (this never returns)
        m_eventThread.runEventLoop();
    }
};

ZTEST(TimerCallbackTests, test_timer_callback_is_called)
{
    LOG_DBG("Starting timer callback test...");
    
    CallbackTestClass testObj;
    
    // Start the timer
    testObj.startTimer();
    
    // Wait for timer to expire (100ms + some buffer)
    k_sleep(K_MSEC(200));
    
    // Verify callback was called
    zassert_equal(testObj.getCallbackCount(), 1, "Callback should have been called once");
    
    // Clean exit
    testObj.exit();
    
    // Wait a bit for clean exit
    k_sleep(K_MSEC(50));
    
    zassert_true(testObj.hasExited(), "Thread should have exited cleanly");
}

class RecurringCallbackTestClass {
public:
    RecurringCallbackTestClass() :
        m_eventThread(
            "RecurringCallbackTest", 
            m_threadStack,
            THREAD_STACK_SIZE,
            [this]() { threadMain(); },
            7,
            EVENT_QUEUE_NUM_ITEMS
        ),
        m_timer("RecurringTimer", [this]() { onTimerCallback(); })
    {
        m_eventThread.timerManager().registerTimer(m_timer);
    }

    void startTimer() {
        m_timer.start(50, 50); // Recurring timer, 50ms period
    }

    void exit() {
        m_eventThread.sendEvent(MyEvents::ExitEvent());
    }

    uint32_t getCallbackCount() const {
        zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
        __ASSERT_NO_MSG(lockGuard.didGetLock());
        return m_callbackCount;
    }

    bool hasExited() const {
        zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
        __ASSERT_NO_MSG(lockGuard.didGetLock());
        return m_hasExited;
    }

    zct::EventThread<MyEvents::Generic> m_eventThread;

private:
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(m_threadStack, THREAD_STACK_SIZE);

    zct::Timer m_timer;
    mutable zct::Mutex m_accessMutex;
    uint32_t m_callbackCount = 0;
    bool m_hasExited = false;

    void onTimerCallback() {
        LOG_DBG("Recurring timer callback called!");
        zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
        __ASSERT_NO_MSG(lockGuard.didGetLock());
        m_callbackCount++;
    }

    void handleEvent(const MyEvents::Generic& event) {
        LOG_DBG("Event received: %d.", event.index());

        if (std::holds_alternative<MyEvents::ExitEvent>(event)) {
            zct::MutexLockGuard lockGuard = m_accessMutex.lockGuard(K_FOREVER);
            __ASSERT_NO_MSG(lockGuard.didGetLock());
            m_hasExited = true;
            LOG_DBG("Exit event received, breaking loop.");
            m_eventThread.exitEventLoop();
        }
    }

    void threadMain() {
        // Register external event callback
        m_eventThread.onExternalEvent([this](const MyEvents::Generic& event) {
            handleEvent(event);
        });

        // Start the event loop (this never returns)
        m_eventThread.runEventLoop();
    }
};

ZTEST(TimerCallbackTests, test_recurring_timer_callback)
{
    LOG_DBG("Starting recurring timer callback test...");
    
    RecurringCallbackTestClass testObj;
    
    // Start the recurring timer
    testObj.startTimer();
    
    // Wait for multiple timer expirations (50ms * 4 + buffer)
    k_sleep(K_MSEC(250));
    
    // Verify callback was called multiple times
    uint32_t callbackCount = testObj.getCallbackCount();
    
    LOG_DBG("Callback count: %d", callbackCount);
    
    // Should have been called at least 3 times (we waited 250ms with 50ms period)
    zassert_true(callbackCount >= 3, "Callback should have been called at least 3 times, got %d", callbackCount);
    
    // Clean exit
    testObj.exit();
    
    // Wait a bit for clean exit
    k_sleep(K_MSEC(50));
    
    zassert_true(testObj.hasExited(), "Thread should have exited cleanly");
}

} // anonymous namespace