cmake_minimum_required(VERSION 2.8)

project(monitor)

set(CMAKE_CXX_FLAGS "-std=c++11 -g")

include_directories(.)
include_directories(./tests)
aux_source_directory(. SRCS)

add_library(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} pthread)

add_executable(coordinator tests/test_coordinator.cpp)
target_link_libraries(coordinator pthread ${PROJECT_NAME})

add_executable(worker tests/test_worker.cpp)
target_link_libraries(worker pthread ${PROJECT_NAME})