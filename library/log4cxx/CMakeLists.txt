cmake_minimum_required(VERSION 3.10)

project(log4cxxtest)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

#LINK_DIRECTORIES('/usr/local/lib')

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} log4cxx)