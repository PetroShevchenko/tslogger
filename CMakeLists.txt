cmake_minimum_required (VERSION 3.5)

set(
    PROJECT1_NAME
        "multithreading"
)

set(
    PROJECT2_NAME
        "container_logging"
)

set(
    PROJECT3_NAME
        "simple_example"
)
project(${PROJECT1_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")

set(
    SRC_DIR
        ${CMAKE_CURRENT_LIST_DIR}/src
)

set(
    INC_DIR
        ${CMAKE_CURRENT_LIST_DIR}/api
)

set(
    EXAMPLES_DIR
        ${CMAKE_CURRENT_LIST_DIR}/examples
)

set(
    SRC_LIST
        ${SRC_DIR}/error.cpp
        ${SRC_DIR}/logger.cpp
        ${INC_DIR}/error.hpp
        ${INC_DIR}/logger.hpp
        ${INC_DIR}/safe_queue.hpp
)

set(
    EXAMPLE1_SRC_LIST
        ${SRC_LIST}
        ${EXAMPLES_DIR}/multithreading.cpp
)

set(
    EXAMPLE2_SRC_LIST
        ${SRC_LIST}
        ${EXAMPLES_DIR}/container_logging.cpp
)

set(
    EXAMPLE3_SRC_LIST
        ${SRC_LIST}
        ${EXAMPLES_DIR}/simple_example.cpp
)

add_executable(
    ${PROJECT1_NAME}
        ${EXAMPLE1_SRC_LIST}
)

add_executable(
    ${PROJECT2_NAME}
        ${EXAMPLE2_SRC_LIST}
)

add_executable(
    ${PROJECT3_NAME}
        ${EXAMPLE3_SRC_LIST}
)

target_include_directories(
    ${PROJECT1_NAME} PRIVATE
        ${INC_DIR}
)

target_include_directories(
    ${PROJECT2_NAME} PRIVATE
        ${INC_DIR}
)

target_include_directories(
    ${PROJECT3_NAME} PRIVATE
        ${INC_DIR}
)

add_definitions(-DUSE_TS_LOGGER)
