cmake_minimum_required(VERSION 3.16)
project(alignment_poc LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(alignment_poc)

set(ALIGNMENT_POC_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
target_compile_definitions(alignment_poc PRIVATE "ROMFS_PATH=\"${ALIGNMENT_POC_ASSETS_DIR}\"")
message(STATUS "Using ROMFS_PATH: ${ALIGNMENT_POC_ASSETS_DIR}")

target_link_libraries(alignment_poc PRIVATE grvl)
target_include_directories(alignment_poc PRIVATE ./)
target_sources(alignment_poc PRIVATE main.cpp)

set_target_properties(alignment_poc PROPERTIES OUTPUT_NAME "alignment_poc")
