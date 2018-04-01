cmake_minimum_required(VERSION 2.7)
project(libprco_top)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(libprco)
add_subdirectory(cli)
add_subdirectory(emu)
