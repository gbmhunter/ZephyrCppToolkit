# Zephyr C++ Toolkit

[![Zephyr CI](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml/badge.svg)](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml)

## Overview

This is a collection of classes and functions that I have found useful when developing Zephyr applications.

## Mutex

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
class MyEventThread : public zct::EventThread {

    MyEventThread(const char* name, int stackSize, int priority) 
        : EventThread(name, stackSize, priority),
        myTimer()
    { 
        // Setup the timer to fire every 1s
        myTimer.start(1000, 1000);
    }

    void threadMain() {
        // Block, waiting for an event. An event can either be:
        // - A internal timer event
        // - An external event (sent from another thread)
        Event event = waitForEvent();

        if (event == Event::MY_TIMER_FIRED) {
            myGpio.toggle();
        } else if (event == Event::START_FLASHING) {
            myTimer.start(1000, 1000);
        }

    }
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