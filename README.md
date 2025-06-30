# Zephyr C++ Toolkit

[![Zephyr CI](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml/badge.svg)](https://github.com/gbmhunter/zephyr-cpp-toolkit/actions/workflows/main.yml)

## Overview

This is a collection of classes and functions that I have found useful when developing Zephyr applications.

Design goals:

* Leverage C++ features to make Zephyr applications easier and safer to write (e.g. RAII based mutex guard, stronger typing with `enum class`, typed unions with `std::variant`).
* Provide an easy way to mock peripherals for testing using `native_sim` and `ztest`.
* Provide a C++ interface over Zephyr peripherals.
* Provide extra functionality on top of what is provided by Zephyr (e.g. event driven threads, timers).

This toolkit uses dynamic memory allocation, but only for initialization. This allows the clean and flexible use of library classes. This means you don't have to

* Pass in pointers to buffers to library classes, they can make their own.
* Use templating to allocate the memory for the buffers.

This should be acceptable for many firmware projects, since once initialization is complete you are safe from the fragmentation and non-determinisitic issues of dynamic memory allocation.

Read the [documentation](https://gbmhunter.github.io/ZephyrCppToolkit/) for more information.

## Installation

This project uses `CMake` to build. To use the library in your project, add the following to your `CMakeLists.txt` file:

```cmake
FetchContent_Declare(
    ZephyrCppToolkit
    GIT_REPOSITORY https://github.com/gbmhunter/ZephyrCppToolkit.git
    GIT_TAG main
)
FetchContent_MakeAvailable(ZephyrCppToolkit)
target_link_libraries(app PRIVATE ZephyrCppToolkit_Real)
```

This project exposes two CMake targets for you to link against in your Zephyr application:

* `ZephyrCppToolkit_Real`: An `INTERFACE` library that bundles the generic code + real implementations of all peripherals.
* `ZephyrCppToolkit_Mock`: An `INTERFACE` library that bundles the generic code + mock implementations of all peripherals.

You can link against either of these targets to get the functionality you need. For example, your real app would link against `ZephyrCppToolkit_Real` and your test application would link against `ZephyrCppToolkit_Mock`.

You can also link against both to get both real and mock implementations.

## Mutex

The Mutex class is a wrapper around the Zephyr mutex API. It provides a RAII style `MutexLockGuard` which automatically unlocks the mutex when it goes out of scope. This reduces the risk of you forgetting to unlock the mutex for all execution paths in your function.

### Usage

Note in the example below how to don't have to remember to unlock the mutex before each `return` statement.

```c++
#include "ZephyrCppToolkit.hpp"

void myFunction() {
    // Creates a Zephyr mutex, starts of unlocked
    zct::Mutex mutex;

    // Lock the mutex
    zct::Mutex mutex;
    zct::MutexLockGuard lockGuard = mutex.lockGuard(K_MSEC(1000));
    __ASSERT(lockGuard.didGetLock(), "Failed to lock the mutex in main thread.");

    // Various things that require the mutex
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

See the [Mutex class documentation](https://gbmhunter.github.io/ZephyrCppToolkit/classzct_1_1Mutex.html) for more information.

## Peripherals

All peripherals are designed so they can be mocked for testing. They follow this pattern:

* `I<peripheral>.hpp`: The interface for the peripheral.
* `<peripheral>Real.hpp/cpp`: The real implementation of the peripheral.
* `<peripheral>Mock.hpp/cpp`: The mock implementation of the peripheral.

All real implementations accept Zephyr device tree structures in their constructors. Mock implementations may additional methods added to them to allow for easier testing (e.g. the `GpioMock` class has a `mockSetInput()` method which allows you to pretend to be the external signal and set the value of an input GPIO).

This pattern is intended to be used as follows:

1. Create an `App` class which contains your application logic. This accepts periperhal interface classes in it's constructor and uses them throughout the application.
1. Your real `main.cpp` creates real peripherals and passes them into your `App` class.
1. Your test code `main.cpp` creates mock peripherals and passes them into your `App` class.

If you have many peripherals the number of individual peripherals passed in can get unweildy. In this case you can create a `IPeripherals` class which wraps all the individual peripherals. Then your `main.cpp` creates either a `PeripheralsReal` or `PeripheralsMock` class and passes that into your `App` class.

See the following documentation for more information:

* [IGpio class](https://gbmhunter.github.io/ZephyrCppToolkit/classzct_1_1IGpio.html)
* [IPwm class](https://gbmhunter.github.io/ZephyrCppToolkit/classzct_1_1IPwm.html)

## Event Thread

EventThread is a base class for easily creating threads following a specific event driven pattern with timers.

One benefit of using this over Zephyrs native timers is that these timers run synchronous with this thread. In Zephyr, timers are run in the system thread, meaning two things:

1. If you want them to interact with other threads, you need to synchonize them (typically by posting of the threads event queue).
1. You can get race conditions in where you receive timer expiry events from another thread after you have apparently stopped it (due to the timer timeout occuring before it was stopped, yet the stopping thread has not processed the item on it's event queue).

Each event thread requires a event type to be defined which is a container of all the possible events that can be sent to the thread via it's internal message queue. All passing of events is done by copy, so you don't have to worry about lifetime issues.

Rather than post to the message queue directly from other modules, it's recommended to create wrapper functions (like the `flash` function below) which do the work of creating the event and posting it to the queue. These functions will be inherently thread safe.

See the [EventThread class documentation](https://gbmhunter.github.io/ZephyrCppToolkit/classzct_1_1EventThread.html) for more information.

## Examples

The `examples/` directory contains some examples of how to use the library.

* `examples/IntegrationTest`: An example of how to use this library to perform integration testing. This tests essentially the entire application, including multiple threads. Mock peripherals are passed into the application. This example also demonstrates the folder structure, with most of the application code in the `src/` directory setup as a CMake `INTERFACE` library, and then two executables defined in the `real/` and the `test/` directories.

## Documentaion

The documentation is generated automatically when there are new commits on the `main` branch. The documentation is hosted on GitHub Pages at [https://gbmhunter.github.io/ZephyrCppToolkit/](https://gbmhunter.github.io/ZephyrCppToolkit/).

To generate the documentation locally (when developing this library), make sure you have `doxygen` installed. Then run:

```bash
cd doxygen-config
doxygen
```

To then serve this locally, run:

```bash
cd doxygen_docs
python3 -m http.server
```

Then navigate to `http://localhost:8000/` in your browser.

The generation is run from the `doxygen-config/` folder rather than the root because I had a weird issue when running it from the root in where I couldn't exclude the `external/` folder from being scanned.

## Unit Tests For This Library

To run the unit tests for this library:

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

The `build.sh` in the root of the repository can be used to build everything and run the unit tests.

```bash
./build.sh
```
