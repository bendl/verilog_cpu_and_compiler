cmake_minimum_required(VERSION 2.7)
project(emu)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -pedantic -Wall -Wextra -Wno-comment")
endif(CMAKE_COMPILER_IS_GNUCXX)

file(GLOB APP_SOURCES *.cpp *.c)
file(GLOB APP_HEADERS *.h)

include_directories(../include)

add_executable(${PROJECT_NAME} ${APP_SOURCES} main.c)

target_link_libraries(${PROJECT_NAME} libprco Shlwapi.lib)

install(TARGETS emu DESTINATION bin)
