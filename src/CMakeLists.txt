cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ZephyrCppToolkit CXX)

# Enable C++23 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Create the library target
# add_library(ZephyrCppToolkit INTERFACE)

# Define two library targets, one for real and one for mock implementations.
add_library(ZephyrCppToolkit_Real INTERFACE)
add_library(ZephyrCppToolkit_Mock INTERFACE)

# Explicitly set the library language to C++
# set_target_properties(ZephyrCppToolkit PROPERTIES
#     LINKER_LANGUAGE CXX)

# target_compile_options(ZephyrCppToolkit INTERFACE -std=c++20)

# Define sources which are common to both real and mock implementations.
set(COMMON_SRC_FILES
    "EventThread/EventThread.cpp"
    "EventThread/Timer.cpp"
    "EventThread/TimerManager.cpp"
    "Gpio/IGpio.cpp"
    "Mutex.cpp"
)

# Add sources to the library
# target_sources(
#     ZephyrCppToolkit
#     INTERFACE
#     ${COMMON_SRC_FILES}
# )

target_sources(
    ZephyrCppToolkit_Real
    INTERFACE
    ${COMMON_SRC_FILES}
    Gpio/GpioReal.cpp
)

target_sources(
    ZephyrCppToolkit_Mock
    INTERFACE
    ${COMMON_SRC_FILES}
    Gpio/GpioMock.cpp
)

# Link against Zephyr interface library to get include paths and other settings
# target_link_libraries(ZephyrCppToolkit INTERFACE zephyr_interface)
target_link_libraries(ZephyrCppToolkit_Real INTERFACE zephyr_interface)
target_link_libraries(ZephyrCppToolkit_Mock INTERFACE zephyr_interface)

# Add compiler option to suppress "warning: structured bindings only available with '-std=c++17' or '-std=gnu++17' [-Wc++17-extensions]"
# target_compile_options(ZephyrCppToolkit PRIVATE -Wno-c++17-extensions)
# target_compile_options(ZephyrCppToolkit PRIVATE -Wno-c++17-extensions)

# Make the library headers available to other targets
# target_include_directories(ZephyrCppToolkit
#     INTERFACE
#     ${CMAKE_CURRENT_SOURCE_DIR}
# )

target_include_directories(ZephyrCppToolkit_Real
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(ZephyrCppToolkit_Mock
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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
