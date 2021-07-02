cmake_minimum_required(VERSION 2.8)

project(monitor)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -fPIC -Wall")

include_directories(.)
include_directories(./tests)
include_directories(./examples)
aux_source_directory(. SRCS)

add_definitions(-DNDEBUG)

add_library(${PROJECT_NAME} STATIC ${SRCS})
target_link_libraries(${PROJECT_NAME} pthread)

option(BUILDEXES "build test programs" OFF)

macro(TEST_BUILD target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} pthread ${PROJECT_NAME})
endmacro()

if (BUILDEXES)
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
endif ()
TEST_BUILD(listener_client
        tests/test_listener_client.cpp)
TEST_BUILD(listener_server
        tests/test_listener_server.cpp)