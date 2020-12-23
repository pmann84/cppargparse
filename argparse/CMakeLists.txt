cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_NAME "cpp_argparse")

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(${PROJECT_NAME} INTERFACE "argparse.h")