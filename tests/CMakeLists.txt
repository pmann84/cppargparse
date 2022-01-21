cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)

project(test_argparse)
set(SOURCES
        basic_tests.cpp
        validation_tests.cpp
        default_argument_tests.cpp
        argument_tests.cpp
        nargs_tests.cpp
        config_file_tests.cpp cascading_input_tests.cpp environment_variable_tests.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(test_argparse PUBLIC argparse)

target_link_libraries(test_argparse PUBLIC gtest_main)
target_link_libraries(test_argparse PUBLIC gmock)
target_link_libraries(test_argparse PUBLIC gmock_main)
target_link_libraries(test_argparse PUBLIC gtest)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/test_config_files $<TARGET_FILE_DIR:test_argparse>/)