cmake_minimum_required(VERSION 3.10)

project(crosscompile)

set(TOOLCHAIN "/home/sun/CrossCompileToolchain")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN}/arm-none-linux-gnueabihf)
set(CMAKE_C_COMPILER ${TOOLCHAIN}/bin/arm-none-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/arm-none-linux-gnueabihf-g++)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRCS)
#add_executable(${PROJECT_NAME} ${SRCS})

add_library(${PROJECT_NAME} SHARED ${SRCS})