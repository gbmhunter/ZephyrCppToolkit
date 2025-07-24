cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ZephyrCppToolkit CXX)

# Define two library targets, one for real and one for mock implementations.
add_library(ZephyrCppToolkit_Real INTERFACE)
add_library(ZephyrCppToolkit_Mock INTERFACE)

# Define sources which are common to both real and mock implementations.
set(COMMON_SRC_FILES
    "Core/Mutex.cpp"
    "Events/EventThread.cpp"
    "Events/Timer.cpp"
    "Events/TimerManager.cpp"
    "Peripherals/IAdc.cpp"
    "Peripherals/IGpio.cpp"
    "Peripherals/IPwm.cpp"
    "Peripherals/IWatchdog.cpp"
)

target_sources(
    ZephyrCppToolkit_Real
    INTERFACE
    ${COMMON_SRC_FILES}
    Peripherals/AdcReal.cpp
    Peripherals/GpioReal.cpp
    Peripherals/PwmReal.cpp
    Peripherals/WatchdogReal.cpp
)

target_sources(
    ZephyrCppToolkit_Mock
    INTERFACE
    ${COMMON_SRC_FILES}
    Peripherals/AdcMock.cpp
    Peripherals/GpioMock.cpp
    Peripherals/PwmMock.cpp
    Peripherals/WatchdogMock.cpp
)

# Link against Zephyr interface library to get include paths and other settings
target_link_libraries(ZephyrCppToolkit_Real INTERFACE zephyr_interface)
target_link_libraries(ZephyrCppToolkit_Mock INTERFACE zephyr_interface)

target_include_directories(ZephyrCppToolkit_Real INTERFACE ../include)
target_include_directories(ZephyrCppToolkit_Mock INTERFACE ../include)

# Disable packaging for TlExpected
set(EXPECTED_BUILD_PACKAGE OFF CACHE BOOL "Disable TlExpected packaging" FORCE)
set(EXPECTED_BUILD_TESTS OFF CACHE BOOL "Disable TlExpected tests" FORCE)

include(FetchContent)
FetchContent_Declare(
    TlExpected
    GIT_REPOSITORY https://github.com/TartanLlama/expected
    GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(TlExpected)

# The target 'expected' (aliased as tl::expected) is now configured.
# Link against the alias provided by the library.
target_link_libraries(ZephyrCppToolkit_Real INTERFACE tl::expected)
target_link_libraries(ZephyrCppToolkit_Mock INTERFACE tl::expected)
