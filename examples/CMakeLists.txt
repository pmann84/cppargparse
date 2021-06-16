cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)

project(example_argparse)
set(SOURCES example_argparse.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(example_argparse PUBLIC argparse)
