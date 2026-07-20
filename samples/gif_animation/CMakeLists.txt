cmake_minimum_required(VERSION 3.16)
project(gif_animation LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(gif_animation)

set(DEMO_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
target_compile_definitions(gif_animation PRIVATE "ROMFS_PATH=\"${DEMO_ASSETS_DIR}\"")
message(STATUS "Using ROMFS_PATH: ${DEMO_ASSETS_DIR}")

target_link_libraries(gif_animation PRIVATE grvl)
target_include_directories(gif_animation PRIVATE ./)
target_sources(gif_animation PRIVATE main.cpp)

set_target_properties(gif_animation PROPERTIES OUTPUT_NAME "demo")
