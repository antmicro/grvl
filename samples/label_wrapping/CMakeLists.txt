cmake_minimum_required(VERSION 3.16)
project(demo LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE grvl)

set(SAMPLE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/romfs")
set(SAMPLE_FONT_PATH "${SAMPLE_ASSETS_DIR}/fonts/Roboto.ttf.gz")
target_compile_definitions(demo PRIVATE
    "ROMFS_PATH=\"${SAMPLE_ASSETS_DIR}\""
    "SAMPLE_FONT_PATH=\"${SAMPLE_FONT_PATH}\"")
