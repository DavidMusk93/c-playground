cmake_minimum_required(VERSION 2.8)

project(monitor)

set(CMAKE_CXX_FLAGS "-std=c++11 -g")

include_directories(.)
include_directories(./tests)
include_directories(./examples)
aux_source_directory(. SRCS)

add_library(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} pthread)

macro(TEST_BUILD target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} pthread ${PROJECT_NAME})
endmacro()

TEST_BUILD(coordinator
        tests/test_coordinator.cpp
        examples/coordinator.cpp)
TEST_BUILD(worker
        tests/test_worker.cpp
        examples/worker.cpp)
TEST_BUILD(reuseport
        tests/test_reuseport.cpp)
TEST_BUILD(forwarder
        examples/forwarder.cpp)