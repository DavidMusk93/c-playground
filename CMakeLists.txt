cmake_minimum_required(VERSION 3.10)

set(my_project c4fun)

project(${my_project})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/tty TTY_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/io IO_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

add_executable(${my_project} ${TTY_SRC} ${IO_SRC})