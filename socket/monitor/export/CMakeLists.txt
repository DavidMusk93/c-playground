cmake_minimum_required(VERSION 2.8)

project(forwarder)

set(MONITOR monitor)
set(CMAKE_CXX_FLAGS "-std=c++11 -g -fPIC")

if ("$ENV{JAVA_HOME}" STREQUAL "")
    message(FATAL_ERROR "environment variable $JAVA_HOME not set")
else ()
    include_directories($ENV{JAVA_HOME}/include)
    include_directories($ENV{JAVA_HOME}/include/linux)
endif ()
add_definitions(-DNOFORWARDERMAIN)

add_subdirectory(.. ${MONITOR})

include_directories(..)
include_directories(../examples)

add_library(${PROJECT_NAME} SHARED export.cpp ../examples/forwarder.cpp)
target_link_libraries(${PROJECT_NAME} ${MONITOR} pthread)