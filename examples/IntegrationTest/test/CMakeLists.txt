cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(IntegrationTestExample_Test)

# Add the IntegrationTestExample library from src directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/IntegrationTestExample_src)

target_sources(
    app
    PRIVATE
    main.cpp
    BasicTests.cpp
)


# Link against the IntegrationTestExample library.
if(NOT TARGET IntegrationTestExample_Lib)
    message(FATAL_ERROR "CMake target 'IntegrationTestExample_Lib' not found. Cannot link to 'app'.")
endif()
target_link_libraries(app PRIVATE IntegrationTestExample_Lib ZephyrCppToolkit_Mock)

# Make the compiler stop at the first error.
target_compile_options(app PRIVATE -Wfatal-errors)