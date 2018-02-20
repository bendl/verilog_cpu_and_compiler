cmake_minimum_required(VERSION 2.7)
project(cli)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")

file(GLOB APP_SOURCES *.cpp *.c)
file(GLOB APP_HEADERS *.h)

include_directories(../include)

add_executable(${PROJECT_NAME} ${APP_SOURCES} main.c)

target_link_libraries(${PROJECT_NAME} libprco Shlwapi.lib)

install(TARGETS cli DESTINATION bin)
