cmake_minimum_required(VERSION 3.20)


project(arena_test)


file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(arena_test ${sources})

target_compile_options(arena_test PUBLIC -fPIC -g -Wall)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(arena_test PUBLIC arena)
