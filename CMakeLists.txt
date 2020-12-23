cmake_minimum_required(VERSION 3.17)

set(PROJECT_NAME "cppargparse")
project(${PROJECT_NAME})


set(CMAKE_CXX_STANDARD 17)

add_subdirectory(argparse)
add_subdirectory(tests)