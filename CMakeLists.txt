cmake_minimum_required (VERSION 3.5)

set(
    PROJECT_NAME
        "multithreading"
)

project(${PROJECT_NAME})

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
        ${EXAMPLES_DIR}/multithreading.cpp
        ${INC_DIR}/error.hpp
        ${INC_DIR}/logger.hpp
        ${INC_DIR}/safe_queue.hpp
)

add_executable(
    ${PROJECT_NAME}
        ${SRC_LIST}
)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
        ${INC_DIR}
)

add_definitions(-DUSE_TS_LOGGER)
