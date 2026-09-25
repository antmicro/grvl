cmake_minimum_required(VERSION 3.16)
project(grvl_demo LANGUAGES CXX)

# This is a separate project as we need the ability to configure the
# grvl library to support Linux while still having that be OFF by default.
# If this was included by the root CMake we would not be able to set
# GRVL_LINUX_NATIVE/GRVL_LINUX_DESKTOP to ON, or enabling demo would force
# those options upon the library as a whole.

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(grvl_demo)

set(SAMPLE_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
set(SAMPLE_FONT_PATH "${CMAKE_SOURCE_DIR}/../input/romfs/fonts/Roboto.ttf.gz")
target_compile_definitions(grvl_demo PRIVATE
    "ROMFS_PATH=\"${SAMPLE_ASSETS_DIR}\""
    "SAMPLE_FONT_PATH=\"${SAMPLE_FONT_PATH}\""
)

target_link_libraries(grvl_demo PRIVATE grvl)
target_include_directories(grvl_demo PRIVATE src/)
target_sources(grvl_demo PRIVATE src/main.cpp)

set_target_properties(grvl_demo PROPERTIES OUTPUT_NAME "demo")
