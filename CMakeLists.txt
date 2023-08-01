cmake_minimum_required(VERSION 3.18.4)
project(grvl LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

set(GRVL_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
option(GRVL_ZEPHYR OFF)


if(ENABLE_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY
  clang-tidy;
  --format-style=file;
  -checks=-*,clang-diagnostic-*,clang-analyzer-*,-*,bugprone-*,performance-*,readability-*,-readability-identifier-length,-readability-braces-around-statements,-readability-implicit-bool-conversion,-bugprone-narrowing-conversions,-readability-function-cognitive-complexity,-bugprone-easily-swappable-parameters,-bugprone-parent-virtual-call,-performance-no-int-to-ptr;
  -warnings-as-errors=*)
  message("enabled clang tidy in build")
endif()

# DEPENDENCIES

  FetchContent_Declare(
          tinyxml2
          GIT_REPOSITORY https://github.com/leethomason/tinyxml2
          GIT_TAG e059560
  )

  FetchContent_GetProperties(tinyxml2)
  if(NOT tinyxml2_POPULATED)
    FetchContent_Populate(tinyxml2)
    message("tinyxml2 src" ${tinyxml2_SOURCE_DIR})
  endif()

  FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_TAG 04f42ce
  )

  FetchContent_GetProperties(zlib)
  if(NOT zlib_POPULATED)
    FetchContent_Populate(zlib)
    message("zlib src" ${zlib_SOURCE_DIR})
  endif()

  FetchContent_Declare(
    libjpeg-turbo
    GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo.git
    GIT_TAG        5829cb2
  )

  FetchContent_GetProperties(libjpeg-turbo)
  if(NOT libjpeg_POPULATED)
    FetchContent_Populate(libjpeg-turbo)
    message("libjpeg-turbo src" ${libjpeg-turbo_SOURCE_DIR})
  endif()


  FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/pnggroup/libpng
    GIT_TAG 8799d78
  )

  FetchContent_GetProperties(libpng)
  if(NOT libpng_POPULATED)
    FetchContent_Populate(libpng)
    message("libpng src" ${libpng_SOURCE_DIR})
  endif()

# DEPENDENCIES END

if(GRVL_ZEPHYR)
  include(${CMAKE_CURRENT_LIST_DIR}/env_support/zephyr.cmake)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/env_support/sdl.cmake)
endif()

set_target_properties(tinyxml2 PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(zlib PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(libjpeg PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(libpng PROPERTIES CXX_CLANG_TIDY "")
