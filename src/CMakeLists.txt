cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ZephyrCppToolkit CXX)

# Enable C++20 standard
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS ON)

# Define two library targets, one for real and one for mock implementations.
add_library(ZephyrCppToolkit_Real INTERFACE)
add_library(ZephyrCppToolkit_Mock INTERFACE)

# Define sources which are common to both real and mock implementations.
set(COMMON_SRC_FILES
    "EventThread/EventThread.cpp"
    "EventThread/EventThreadv2.cpp"
    "EventThread/Timer.cpp"
    "EventThread/TimerManager.cpp"
    "Gpio/IGpio.cpp"
    "Mutex.cpp"
    "Pwm/IPwm.cpp"
)

target_sources(
    ZephyrCppToolkit_Real
    INTERFACE
    ${COMMON_SRC_FILES}
    Gpio/GpioReal.cpp
    Pwm/PwmReal.cpp
)

target_sources(
    ZephyrCppToolkit_Mock
    INTERFACE
    ${COMMON_SRC_FILES}
    Gpio/GpioMock.cpp
    Pwm/PwmMock.cpp
)

# Link against Zephyr interface library to get include paths and other settings
# target_link_libraries(ZephyrCppToolkit INTERFACE zephyr_interface)
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
# target_link_libraries(ZephyrCppToolkit INTERFACE tl::expected)
target_link_libraries(ZephyrCppToolkit_Real INTERFACE tl::expected)
target_link_libraries(ZephyrCppToolkit_Mock INTERFACE tl::expected)
