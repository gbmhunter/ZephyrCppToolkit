# Zephyr C++ Toolkit

[![Zephyr CI](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml/badge.svg)](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml)

## Overview

This is a collection of classes and functions that I have found useful when developing Zephyr applications.

Design goals:

* Leverage C++ features to make Zephyr applications easier and safer to write (the RAII based mutex guard is a good example of this)
* Provide an easy way to mock peripherals for testing
* Provide common functionality ontop of what is provided by Zephyr that may be useful to applications

This toolkit uses dynamic memory allocation, but only for initialization. This allows the clean and flexible use of library classes. This means you don't have to

* Pass in pointers to buffers to library classes, they can make their own.
* Use templating to allocate the memory for the buffers.

This should be acceptable for many firmware projects, since once initialization is complete you are safe from the fragmentation and non-determinisitic issues of dynamic memory allocation.

## Installation

This project exposes two CMake targets:

* `ZephyrCppToolkit_Real`: An `INTERFACE` library that bundles the generic code + real implementations of all peripherals.
* `ZephyrCppToolkit_Mock`: An `INTERFACE` library that bundles the generic code + mock implementations of all peripherals.

You can link against either of these targets to get the functionality you need. For example, your real app would link against `ZephyrCppToolkit_Real` and your test application would link against `ZephyrCppToolkit_Mock`.

You can also link against both to get both real and mock implementations.

To use the library in your project, add the following to your `CMakeLists.txt`:

```cmake
target_link_libraries(app PRIVATE ZephyrCppToolkit_Real)
```

## Mutex

The Mutex class is a wrapper around the Zephyr mutex API. It provides a RAII style `MutexLockGuard` which automatically unlocks the mutex when it goes out of scope. This reduces the risk of you forgetting to unlock the mutex for all execution paths in your function.

### Usage

```c++
#include "ZephyrCppToolkit.hpp"

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

## Peripherals

All peripherals are designed so they can be mocked for testing. They follow this pattern:

* `I<peripheral>.hpp`: The interface for the peripheral.
* `<peripheral>Real.hpp/cpp`: The real implementation of the peripheral.
* `<peripheral>Mock.hpp/cpp`: The mock implementation of the peripheral.

All real implementations accept Zephyr device tree structures in their constructors.

This pattern is intended to be used as follows:

1. Create an `App.cpp` class which contains your application logic. This accepts periperhal interface classes in it's constructor and uses them throughout the application.
1. Your real `main.cpp` creates real peripherals and passes them into your `App` class.
1. Your test code `main.cpp` creates mock peripherals and passes them into your `App` class.

If you have many peripherals the number of individual peripherals passed in can get unweildy. In this case you can create a `IPeripherals` class which wraps all the individual peripherals. Then your `main.cpp` creates either a `PeripheralsReal` or `PeripheralsMock` class and passes that into your `App` class.

### GPIO 

The `zct::GpioReal` class provides a C++ interface to Zephyr GPIOs. It is designed to be used as follows:

```c++
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Peripherals/GpioReal.hpp"

LOG_MODULE_REGISTER(GpioExample, LOG_LEVEL_DBG);

static const struct gpio_dt_spec l_inputGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(example_gpios, input_gpio), gpios);
static const struct gpio_dt_spec l_outputGpioSpec = GPIO_DT_SPEC_GET(DT_PATH(example_gpios, output_gpio), gpios);

int main() {

    // Create an input GPIO. By default they are set to be an input, active high.
    // The name is used for logging purposes.
    zct::GpioReal myInput("MyInput", &l_inputGpioSpec);

    // Create an output GPIO.
    zct::GpioReal myOutput("MyOutput", &l_outputGpioSpec, zct::IGpio::Direction::Output);

    while (true) {
        // Read logical value of input GPIO.
        bool inputValue = myInput.get();
        LOG_INF("Input value: %d", inputValue);

        // Set logical value of output GPIO.
        myOutput.set(!inputValue);
    }
}
```

## Event Thread

EventThread is a base class for easily creating threads following a specific event driven pattern with timers.

One benefit of using this over Zephyrs native timers is that these timers run synchronous with this thread. In Zephyr, timers are run in the system thread, meaning two things:

1. If you want them to interact with other threads, you need to synchonize them (typically by posting of the threads event queue).
1. You can get race conditions in where you receive timer expiry events from another thread after you have apparently stopped it (due to the timer timeout occuring before it was stopped, yet the stopping thread has not processed the item on it's event queue).

Each event thread requires a event type to be defined which is a container of all the possible events that can be sent to the thread via it's internal message queue. All passing of events is done by copy, so you don't have to worry about lifetime issues.

Rather than post to the message queue directly from other modules, it's recommended to create wrapper functions (like the `flash` function below) which do the work of creating the event and posting it to the queue. These functions will be inherently thread safe.

```c++
#include <variant>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/EventThread.hpp"

LOG_MODULE_REGISTER(EventThreadTests, LOG_LEVEL_DBG);

//================================================================================================//
// EVENTS
//================================================================================================//

namespace Events {

struct MyTimerExpiry {
};

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

class Led : public zct::EventThread<Events::Generic> {
public:
    Led() :
        zct::EventThread<Events::Generic>("Led", threadStack, THREAD_STACK_SIZE, EVENT_QUEUE_NUM_ITEMS),
        m_flashingTimer(Events::MyTimerExpiry())
    {
        // Register timers
        m_timerManager.registerTimer(m_flashingTimer);
    }

    ~Led() {
        // Send the exit event to the event thread
        Events::Exit exitEvent;
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
        Events::LedFlashing ledFlashingEvent = { .flashRateMs = flashRateMs };
        sendEvent(ledFlashingEvent);
    }

private:
    static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
    static constexpr size_t THREAD_STACK_SIZE = 512;
    K_KERNEL_STACK_MEMBER(threadStack, THREAD_STACK_SIZE);
    zct::Timer<Events::Generic> m_flashingTimer;
    bool m_ledIsOn = false;

    void threadMain() override {
        while (1) {
            Events::Generic event = zct::EventThread<Events::Generic>::waitForEvent();
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