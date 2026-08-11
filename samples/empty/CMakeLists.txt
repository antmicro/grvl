cmake_minimum_required(VERSION 3.20)
project(grvl_demo LANGUAGES CXX)

if (GRVL_ZEPHYR)
    target_sources(app PRIVATE demo.cpp)
    return()
endif()

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(grvl_demo)

set(GRVL_DEMO_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
target_compile_definitions(grvl_demo PRIVATE "ROMFS_PATH=\"${GRVL_DEMO_ASSETS_DIR}\"")
message(STATUS "Using ROMFS_PATH: ${GRVL_DEMO_ASSETS_DIR}")

target_link_libraries(grvl_demo PRIVATE grvl)
target_include_directories(grvl_demo PRIVATE ./)
target_sources(grvl_demo PRIVATE demo.cpp)

set_target_properties(grvl_demo PROPERTIES OUTPUT_NAME "demo")
