cmake_minimum_required(VERSION 3.16)
project(grvl_drm_lease LANGUAGES CXX)

set(GRVL_LINUX_NATIVE ON CACHE BOOL "" FORCE)

add_subdirectory("../../" grvl)
add_executable(grvl_drm_lease)

set(SAMPLE_ASSETS_DIR "${CMAKE_SOURCE_DIR}/romfs")
set(SAMPLE_FONT_PATH "${CMAKE_SOURCE_DIR}/../input/romfs/fonts/Roboto.ttf.gz")
target_compile_definitions(grvl_drm_lease PRIVATE
    "ROMFS_PATH=\"${SAMPLE_ASSETS_DIR}\""
    "SAMPLE_FONT_PATH=\"${SAMPLE_FONT_PATH}\""
)
message(STATUS "Using ROMFS_PATH: ${SAMPLE_ASSETS_DIR}")
message(STATUS "Using sample font: ${SAMPLE_FONT_PATH}")

target_link_libraries(grvl_drm_lease PRIVATE grvl)
target_sources(grvl_drm_lease PRIVATE src/main.cpp)

set_target_properties(grvl_drm_lease PROPERTIES OUTPUT_NAME "demo")