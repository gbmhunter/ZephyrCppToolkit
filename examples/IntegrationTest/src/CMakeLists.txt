cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(IntegrationTestExample_Lib)

# Add the ZephyrCppToolkit library from src directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../src ${CMAKE_CURRENT_BINARY_DIR}/ZephyrCppToolkit_src)


add_library(IntegrationTestExample_Lib INTERFACE)

target_sources(
    IntegrationTestExample_Lib
    INTERFACE
    App.cpp
)

target_include_directories(
    IntegrationTestExample_Lib
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
