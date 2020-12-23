cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_NAME "test_cpp_argparse")
set(SOURCES basic_tests.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC cpp_argparse)