cmake_minimum_required(VERSION 3.16)
project(char_bounding LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(char_bounding)

set(CHAR_BOUNDING_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
target_compile_definitions(char_bounding PRIVATE "ROMFS_PATH=\"${CHAR_BOUNDING_ASSETS_DIR}\"")
message(STATUS "Using ROMFS_PATH: ${CHAR_BOUNDING_ASSETS_DIR}")

target_link_libraries(char_bounding PRIVATE grvl)
target_include_directories(char_bounding PRIVATE ./)
target_sources(char_bounding PRIVATE main.cpp)

set_target_properties(char_bounding PROPERTIES OUTPUT_NAME "demo")
