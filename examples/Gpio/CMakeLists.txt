cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(GpioExample)

# Add the library from src directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src ${CMAKE_CURRENT_BINARY_DIR}/src)

target_sources(
    app
    PRIVATE
    main.cpp
)
    
# Link against the real library.
if(NOT TARGET ZephyrCppToolkit_Real)
    message(FATAL_ERROR "CMake target 'ZephyrCppToolkit_Real' not found. Cannot link to 'app'.")
endif()
target_link_libraries(app PRIVATE ZephyrCppToolkit_Real)

# Make the compiler stop at the first error.
target_compile_options(app PRIVATE -Wfatal-errors)