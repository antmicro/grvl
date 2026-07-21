cmake_minimum_required(VERSION 3.16)
project(demo LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(demo)

target_compile_definitions(demo PRIVATE "ROMFS_PATH=\"${CMAKE_SOURCE_DIR}/romfs\"")
target_link_libraries(demo PRIVATE grvl)
target_sources(demo PRIVATE main.cpp)
