cmake_minimum_required(VERSION 3.20.0)

# This is used by the C/C++ extension in VSCode to provide intellisense for the project.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr_cpp_toolkit CXX)

# Enable C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler flags for architecture and alignment
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new -m32")

# Create the library target
add_library(zephyr_cpp_toolkit STATIC)

# Explicitly set the library language to C++
set_target_properties(zephyr_cpp_toolkit PROPERTIES
    LINKER_LANGUAGE CXX)

# Add sources to the library
target_sources(
    zephyr_cpp_toolkit
    PRIVATE
    Mutex.cpp
)

# Add compiler option to suppress "warning: structured bindings only available with '-std=c++17' or '-std=gnu++17' [-Wc++17-extensions]"
target_compile_options(zephyr_cpp_toolkit PRIVATE -Wno-c++17-extensions)

# Make the library headers available to other targets
target_include_directories(zephyr_cpp_toolkit
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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