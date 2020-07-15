cmake_minimum_required(VERSION 3.10)

set(my_project c4fun)

project(${my_project})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/tty TTY_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/io IO_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/pty PTY_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/multicast MULTICAST_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/crc CRC_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/time TIME_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/trivial TRIVIAL_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/num NUM_SRC)
aux_source_directory(container CONTAINER_SRC)
aux_source_directory(algorithm ALGORITHM_SRC)
aux_source_directory(class CLASS_SRC)
aux_source_directory(ptrace PTRACE_SRC)
aux_source_directory(builtin BUILTIN_SRC)
aux_source_directory(process PROCESS_SRC)
aux_source_directory(gcc GCC_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

add_executable(${my_project}
        ${TTY_SRC} ${IO_SRC} ${PTY_SRC} ${MULTICAST_SRC}
        ${CRC_SRC} ${TIME_SRC} ${TRIVIAL_SRC} ${CLASS_SRC}
        ${NUM_SRC} ${CONTAINER_SRC} ${ALGORITHM_SRC} ${PTRACE_SRC}
        ${BUILTIN_SRC} ${PROCESS_SRC} ${GCC_SRC})