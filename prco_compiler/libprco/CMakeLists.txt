cmake_minimum_required(VERSION 2.7)
project(libprco)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")

include_directories(../include/${PROJECT_NAME})
include_directories("C:/Program Files (x86)/libprco/include")

file(GLOB PRCO_SOURCES
        *.c
        adt/*.c
        arch/*.c)

file(GLOB PRCO_HEADERS
    ../include/libprco/*.h
    ../include/libprco/adt/*.h
    ../include/libprco/arch/*.h)

add_library(${PROJECT_NAME} STATIC ${PRCO_SOURCES} ${PRCO_HEADERS} module.c ../include/libprco/module.h)

install(TARGETS libprco DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)

