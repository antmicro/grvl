cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(grvl-demo)

set(GRVL_ZEPHYR ON CACHE BOOL "" FORCE)
add_subdirectory("../" demo)
