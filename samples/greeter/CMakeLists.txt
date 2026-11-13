cmake_minimum_required(VERSION 3.16)
project(sample LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)
set(GRVL_LINUX_DESKTOP ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(sample)

set(ALIGNMENT_POC_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
target_compile_definitions(sample PRIVATE "ROMFS_PATH=\"${ALIGNMENT_POC_ASSETS_DIR}\"")
message(STATUS "Using ROMFS_PATH: ${ALIGNMENT_POC_ASSETS_DIR}")

target_link_libraries(sample PRIVATE grvl)
target_include_directories(sample PRIVATE ./)
target_sources(sample PRIVATE main.cpp)

set_target_properties(sample PROPERTIES OUTPUT_NAME "demo")

execute_process(
    COMMAND git rev-parse --short=8 HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT GIT_HASH)
    set(GIT_HASH "UNKNOWN")
endif()

string(TOUPPER "${GIT_HASH}" GIT_HASH)
target_compile_definitions(sample PRIVATE GIT_HASH="${GIT_HASH}")
