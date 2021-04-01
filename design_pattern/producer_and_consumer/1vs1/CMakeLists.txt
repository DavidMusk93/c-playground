cmake_minimum_required(VERSION 2.8)

project(1vs1)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11")

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} pthread)