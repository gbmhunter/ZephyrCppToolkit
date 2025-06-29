cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
# zephyr_library_compile_options(-std=c++23)

project(test)

# Enable C++23 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Add compiler flag for aligned new operator
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")

# Add compiler option to suppress "warning: structured bindings only available with '-std=c++17' or '-std=gnu++17' [-Wc++17-extensions]"

# Add the library from src directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/src)

target_sources(
    app
    PRIVATE
    main.cpp
    MutexTests.cpp
    )
    
# Link against the library
target_link_libraries(app PRIVATE zephyr_cpp_toolkit)
    
target_compile_options(app PRIVATE -Wno-c++17-extensions -std=c++20)
target_compile_options(app PRIVATE -std=c++20)
# Add src/ as an include directory (not needed anymore since the library provides its include paths)
# target_include_directories(app PUBLIC ../src)

zephyr_library_compile_options(-std=c++20)
