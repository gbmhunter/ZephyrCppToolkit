cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr_cpp_toolkit CXX)

# Enable C++23 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Add compiler flags for architecture and alignment
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new -m32")

# Create the library target
add_library(zephyr_cpp_toolkit STATIC)

# Explicitly set the library language to C++
set_target_properties(zephyr_cpp_toolkit PROPERTIES
    LINKER_LANGUAGE CXX)

target_compile_options(zephyr_cpp_toolkit PRIVATE -std=c++20)

# Add sources to the library
target_sources(
    zephyr_cpp_toolkit
    PRIVATE
    EventThread/EventThread.cpp
    EventThread/Timer.cpp
    EventThread/TimerManager.cpp
    Gpio/GpioReal.cpp
    Mutex.cpp
)

# Link against Zephyr interface library to get include paths and other settings
target_link_libraries(zephyr_cpp_toolkit PUBLIC zephyr_interface zephyr)

# Add compiler option to suppress "warning: structured bindings only available with '-std=c++17' or '-std=gnu++17' [-Wc++17-extensions]"
# target_compile_options(zephyr_cpp_toolkit PRIVATE -Wno-c++17-extensions)
# target_compile_options(zephyr_cpp_toolkit PRIVATE -Wno-c++17-extensions)

# Make the library headers available to other targets
target_include_directories(zephyr_cpp_toolkit
    PUBLIC
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
target_link_libraries(zephyr_cpp_toolkit PUBLIC tl::expected)


# include(FetchContent)
# set(NINJAHSM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# FetchContent_Declare(NinjaHSM
#     GIT_REPOSITORY https://github.com/gbmhunter/NinjaHSM.git
#     GIT_TAG 85146685c89a17f14be2b86734413a755b7c9218 # This can be a hash, tag or branch.
# )
# FetchContent_MakeAvailable(NinjaHSM)

# target_link_libraries(app PUBLIC NinjaHsm)
# target_include_directories(app PUBLIC ${NinjaHSM_SOURCE_DIR}/include)

# Add src/ as an include directory
# target_include_directories(app PUBLIC src)