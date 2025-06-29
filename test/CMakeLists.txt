cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(test)

# Add the library from src directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/src)

target_sources(
    app
    PRIVATE
    main.cpp
    EventThreadTests.cpp
    MutexTests.cpp
)
    
# Link against the library
target_link_libraries(app PRIVATE zephyr_cpp_toolkit)
