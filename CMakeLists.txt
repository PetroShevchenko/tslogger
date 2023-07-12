cmake_minimum_required (VERSION 3.5)

set(
    PROJECT_NAME
        "tslogger"
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
        ${INC_DIR}/error.hpp
        ${INC_DIR}/logger.hpp
        ${INC_DIR}/safe_queue.hpp
)

add_definitions(-DUSE_TS_LOGGER)

add_library(
    ${PROJECT_NAME} STATIC
        ${SRC_LIST}
)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
        ${INC_DIR}
)

##############################################################
# Examples
##############################################################

set(
    EXAMPLE1_NAME
        "simple_example"
)

set(
    EXAMPLE2_NAME
        "multithreading"
)

set(
    EXAMPLE3_NAME
        "container_logging"
)

set(
    EXAMPLE1_SRC_LIST
        ${EXAMPLES_DIR}/simple_example.cpp
)

set(
    EXAMPLE2_SRC_LIST
        ${EXAMPLES_DIR}/multithreading.cpp
)

set(
    EXAMPLE3_SRC_LIST
        ${EXAMPLES_DIR}/container_logging.cpp
)

add_executable(
    ${EXAMPLE1_NAME}
        ${EXAMPLE1_SRC_LIST}
)

add_executable(
    ${EXAMPLE2_NAME}
        ${EXAMPLE2_SRC_LIST}
)

add_executable(
    ${EXAMPLE3_NAME}
        ${EXAMPLE3_SRC_LIST}
)

target_link_libraries(
    ${EXAMPLE1_NAME}
        tslogger
)

target_link_libraries(
    ${EXAMPLE2_NAME}
        tslogger
)

target_link_libraries(
    ${EXAMPLE3_NAME}
        tslogger
)

target_include_directories(
    ${EXAMPLE1_NAME} PRIVATE
        ${INC_DIR}
)

target_include_directories(
    ${EXAMPLE2_NAME} PRIVATE
        ${INC_DIR}
)

target_include_directories(
    ${EXAMPLE3_NAME} PRIVATE
        ${INC_DIR}
)
