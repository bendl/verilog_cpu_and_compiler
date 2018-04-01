cmake_minimum_required(VERSION 2.7)
project(libprco)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -pedantic -Wall -Wextra -Wno-comment")
endif(CMAKE_COMPILER_IS_GNUCXX)

include_directories(../include/${PROJECT_NAME})

file(GLOB PRCO_SOURCES
        *.c
        adt/*.c
        arch/*.c)

file(GLOB PRCO_HEADERS
    ../include/libprco/*.h
    ../include/libprco/adt/*.h
    ../include/libprco/arch/*.h)

add_library(${PROJECT_NAME} STATIC ${PRCO_SOURCES} ${PRCO_HEADERS})

install(TARGETS libprco DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)

