cmake_minimum_required(VERSION 3.10)
project(cpp4fun)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)

add_executable(cpp4fun ${SOURCE_FILES} virtual-dispatcher.cpp)