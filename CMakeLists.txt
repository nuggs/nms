cmake_minimum_required(VERSION 2.8)

project("Nuggs Merc MUD")

#add_definitions(-Wall -std=c11 -02)
add_definitions(-Wall -std=c11 -ggdb)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../src)

add_subdirectory(src)

