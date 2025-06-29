cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(IntegrationTestExample_App)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/IntegrationTestExample_Lib)

target_sources(
    app # This is the Zephyr application target
    PRIVATE
    main.cpp
)

target_compile_options(app PRIVATE -Wfatal-errors -std=c++20)

# Link the executable 'app' against our library 'app_lib'
target_link_libraries(app PRIVATE IntegrationTestExample_Lib ZephyrCppToolkit_Real)