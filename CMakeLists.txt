cmake_minimum_required(VERSION 3.20)

project(arena)

file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB arena_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

message(${arena_files})

add_executable(arena_e ${arena_files})
target_compile_options(arena_e PRIVATE -g -Wall)


list(FILTER arena_files EXCLUDE REGEX "main.c")

add_library(arena SHARED ${arena_files})
add_library(arena_static STATIC ${arena_files})

target_compile_options(arena PUBLIC -fPIC)
target_compile_options(arena_static PUBLIC -fPIC)


# Debug
# target_compile_options(arena PUBLIC -fPIC -g -Wall -pg)
# target_compile_options(arena_static PUBLIC -fPIC -g -Wall -pg)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(
  arena
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  arena_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  arena_e
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


    include(FetchContent)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)



set(LIBRARIES m)

target_link_libraries(arena_e PRIVATE ${LIBRARIES})
target_link_libraries(arena PRIVATE ${LIBRARIES})
target_link_libraries(arena_static PRIVATE ${LIBRARIES})
