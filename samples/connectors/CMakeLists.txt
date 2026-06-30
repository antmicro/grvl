cmake_minimum_required(VERSION 3.16)
project(grvl_connectors LANGUAGES CXX)

# This is Linux Native specific sample
set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(grvl_connectors)

set(SAMPLE_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
target_compile_definitions(grvl_connectors PRIVATE "ROMFS_PATH=\"${SAMPLE_ASSETS_DIR}\"")
target_compile_definitions(grvl_connectors PRIVATE GRVL_LINUX_NATIVE_SUPPORT)
message(STATUS "Using ROMFS_PATH: ${SAMPLE_ASSETS_DIR}")

target_link_libraries(grvl_connectors PRIVATE grvl)
target_include_directories(grvl_connectors PRIVATE ./)
target_sources(grvl_connectors PRIVATE src/main.cpp)

set_target_properties(grvl_connectors PROPERTIES OUTPUT_NAME "demo")
