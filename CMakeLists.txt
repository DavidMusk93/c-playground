cmake_minimum_required(VERSION 3.10)

project(c4fun)

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
aux_source_directory(structure STRUCTURE_SRC)
aux_source_directory(socket SOCKET_SRC)
aux_source_directory(meta META_SRC)
aux_source_directory(functional FUNCTIONAL_SRC)
aux_source_directory(feature FEATURE_SRC)
aux_source_directory(db DB_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/socket)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/db)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/io)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        ${TTY_SRC} ${IO_SRC} ${PTY_SRC} ${MULTICAST_SRC}
        ${CRC_SRC} ${TIME_SRC} ${TRIVIAL_SRC} ${CLASS_SRC}
        ${NUM_SRC} ${CONTAINER_SRC} ${ALGORITHM_SRC} ${PTRACE_SRC}
        ${BUILTIN_SRC} ${PROCESS_SRC} ${GCC_SRC} ${STRUCTURE_SRC}
        ${SOCKET_SRC} ${META_SRC} ${FUNCTIONAL_SRC} ${FEATURE_SRC}
        ${DB_SRC})

target_link_libraries(${PROJECT_NAME} pthread)