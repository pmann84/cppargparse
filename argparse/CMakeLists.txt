cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)

add_library(argparse INTERFACE)

target_include_directories(argparse INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(argparse INTERFACE "argparse.h")