cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)

project(test_argparse)
set(SOURCES basic_tests.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(test_argparse PUBLIC argparse)

target_link_libraries(test_argparse PUBLIC gtest_main)
target_link_libraries(test_argparse PUBLIC gmock)
target_link_libraries(test_argparse PUBLIC gmock_main)
target_link_libraries(test_argparse PUBLIC gtest)
