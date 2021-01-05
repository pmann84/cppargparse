cmake_minimum_required(VERSION 3.17)
project(cppargparse)

set(CMAKE_CXX_STANDARD 17)

# Disable the ZERO_CHECK project generation
set(CMAKE_SUPPRESS_REGENERATION true)

# Since googletest uses these flags we should too.
#SET(CMAKE_CXX_FLAGS_DEBUG "/MTd")
#SET(CMAKE_CXX_FLAGS_RELEASE "/MT")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Od")

add_subdirectory(vendor)
add_subdirectory(argparse)
add_subdirectory(tests)