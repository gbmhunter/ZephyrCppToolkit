# Zephyr C++ Toolkit

[![Zephyr CI](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml/badge.svg)](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml)

## Overview

This is a collection of classes and functions that I have found useful when developing Zephyr applications.

## Mutex

The Mutex class is a wrapper around the Zephyr mutex API. It provides a RAII style `MutexLockGuard` which automatically unlocks the mutex when it goes out of scope. This reduces the risk of you forgetting to unlock the mutex for all execution paths in your function.

### Usage

```c++
#include "Mutex.hpp"

namespace zct = zephyr_cpp_toolkit;

void myFunction() {
    // Creates a Zephyr mutex, starts of unlocked
    zct::Mutex mutex;

    // Lock the mutex
    int mutexRc;
    auto lockGuard = zct::MutexLockGuard(mutex, K_FOREVER, mutexRc);
    __ASSERT(mutexRc == 0, "Failed to lock the mutex in main thread.");

    // 
    if (uartDriverNotReady) {
        return;
    }

    if (failedToSendByte) {
        return;
    }


    if (didNotGetResponse) {
        return;
    }

    return;
}

int main() {
    myFunction();
    // The lock guard guarantees that the mutex is unlocked when myFunction returns
}
```

## Event Thread

EventThread is a base class for easily creating threads following a specific event driven pattern with timers.

One benefit of using this over Zephyrs native timers is that these timers run synchronous with this thread. In Zephyr, timers are run in the system thread, meaning two things:

1. If you want them to interact with other threads, you need to synchonize them (typically by posting of the threads event queue).
1. You can get race conditions in where you receive timer expiry events from another thread after you have apparently stopped it (due to the timer timeout occuring before it was stopped, yet the stopping thread has not processed the item on it's event queue).

```c++
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

class Led : public zct::EventThread<Event> {
public:
    Led() :
        zct::EventThread<Event>(threadStack, THREAD_STACK_SIZE, EVENT_QUEUE_NUM_ITEMS),
        m_flashingTimer()
    {
        // Register timers
        m_timerManager.registerTimer(m_flashingTimer);
    }

    ~Led() {
        // Send the exit event to the event thread
        ExitEvent exitEvent;
        sendEvent(exitEvent);
    }

    /**
     * Start flashing the LED at the given rate.
     * 
     * THREAD SAFE.
     * 
     * @param flashRateMs The rate at which to flash the LED.
     */
    void flash(uint32_t flashRateMs) {
        LedFlashingEvent ledFlashingEvent = { .flashRateMs = flashRateMs };
        sendEvent(ledFlashingEvent);
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
            if (std::holds_alternative<MyTimerTimeoutEvent>(event)) {
                LOG_INF("Toggling LED to %d.", !m_ledIsOn);
                m_ledIsOn = !m_ledIsOn;
            } else if (std::holds_alternative<LedFlashingEvent>(event)) {
                // Start the timer to flash the LED
                m_flashingTimer.start(1000, 1000);
                LOG_INF("Starting flashing. Turning LED on...");
                m_ledIsOn = true;
            } else if (std::holds_alternative<ExitEvent>(event)) {
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
```

## Tests

To run the tests:

```bash
cd test
west build -b native_sim
./build/test/zephyr/zephyr.exe
```

To run a specific test, e.g. all tests in the `EventThreadTests` suite:

```bash
cd test
west build -b native_sim && ./build/test/zephyr/zephyr.exe --test="EventThreadTests::*"
```