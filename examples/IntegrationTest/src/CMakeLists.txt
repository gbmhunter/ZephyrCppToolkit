cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(IntegrationTestExample)

# Add the ZephyrCppToolkit library from src directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../src ${CMAKE_CURRENT_BINARY_DIR}/ZephyrCppToolkit_src)


add_library(IntegrationTestExample_Lib INTERFACE)

target_sources(
    IntegrationTestExample_Lib
    INTERFACE
    other.cpp
)

#===================================================================================================
# Real App Executable Definition (app)
#===================================================================================================

# This is the real Zephyr application target.
# It is defined if this CMakeLists.txt is the top-level project.
# It is not defined if this CMakeLists.txt is a subdirectory.
#  This is so that this file can be added via add_subdirectory() in the test application.

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Zephyr's build system will set this up as an executable using the sources defined below.

    target_sources(
        app # This is the Zephyr application target
        PRIVATE
        src/main.cpp
        src/Peripherals/PeripheralsReal.cpp # Added here
    )

    target_compile_options(app PRIVATE -Wfatal-errors -std=c++20)

    # Link the executable 'app' against our library 'app_lib'
    target_link_libraries(app PRIVATE IntegrationTestExample_Lib ZephyrCppToolkit_Real)
endif()