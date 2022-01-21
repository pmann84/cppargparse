cmake_minimum_required(VERSION 3.18)
project(cppargparse
        VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)

# Disable the ZERO_CHECK project generation
set(CMAKE_SUPPRESS_REGENERATION true)

# Since googletest uses these flags we should too.
#SET(CMAKE_CXX_FLAGS_DEBUG "/MTd")
#SET(CMAKE_CXX_FLAGS_RELEASE "/MT")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Od")

add_library(argparse INTERFACE)
target_include_directories(argparse INTERFACE argparse/)

add_subdirectory(vendor)
add_subdirectory(tests)
add_subdirectory(examples)