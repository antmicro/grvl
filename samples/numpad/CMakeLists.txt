cmake_minimum_required(VERSION 3.16)
project(demo LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE grvl)

target_compile_definitions(demo PRIVATE
    "ROMFS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/romfs\""
    "DEMO_FONT_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/romfs/fonts/Roboto.ttf.gz\"")
set_target_properties(demo PROPERTIES OUTPUT_NAME demo)
