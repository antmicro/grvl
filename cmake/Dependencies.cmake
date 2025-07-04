if(USE_SYSTEM_LIBRARIES)
  find_package(ZLIB REQUIRED)
  find_package(JPEG REQUIRED)
  find_package(PNG REQUIRED)
  find_package(tinyxml2 REQUIRED)
  find_package(duktape REQUIRED)

  add_library(ZLIB ALIAS ZLIB::ZLIB)
  add_library(JPEG ALIAS JPEG::JPEG)
  add_library(PNG ALIAS PNG::PNG)

  return()
endif()

include(CPM)

# Set minimum CMake Policy Version for dependencies
set(CMAKE_POLICY_VERSION_MINIMUM 3.16)

# Zlib
CPMAddPackage(
  NAME ZLIB
  VERSION 2.0.6
  GITHUB_REPOSITORY zlib-ng/zlib-ng
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE
  GIT_TAG 2.0.6
  EXCLUDE_FROM_ALL TRUE
  DOWNLOAD_ONLY ${GRVL_ZEPHYR}
  OPTIONS
    "ZLIB_COMPAT ON"
    "ZLIB_ENABLE_TESTS OFF")

if(NOT GRVL_ZEPHYR)
  add_library(ZLIB ALIAS zlib)
endif()

# libjpeg
if(GRVL_STATIC OR NOT BUILD_SHARED_LIBS)
  set(LIBJPEG_OPTIONS
    "ENABLE_SHARED OFF"
    "CMAKE_POSITION_INDEPENDENT_CODE ON")
else()
  set(LIBJPEG_OPTIONS
    "ENABLE_SHARED ON")
endif()

add_compile_definitions(ENTROPY_OPT_SUPPORTED)

CPMAddPackage(
  NAME JPEG
  VERSION 2.0.2
  GITHUB_REPOSITORY libjpeg-turbo/libjpeg-turbo
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE
  GIT_TAG 2.0.2
  EXCLUDE_FROM_ALL TRUE
  DOWNLOAD_ONLY ${GRVL_ZEPHYR}
  OPTIONS
    "WITH_TURBOJPEG OFF"
    ${LIBJPEG_OPTIONS})

if(NOT GRVL_ZEPHYR)
  add_library(JPEG INTERFACE)
  target_include_directories(JPEG INTERFACE ${JPEG_SOURCE_DIR} ${JPEG_BINARY_DIR})

  if(GRVL_STATIC OR NOT BUILD_SHARED_LIBS)
    target_link_libraries(JPEG INTERFACE jpeg-static)
  else()
    target_link_libraries(JPEG INTERFACE jpeg)
  endif()
endif()

# libpng
if(GRVL_STATIC OR NOT BUILD_SHARED_LIBS)
  set(LIBPNG_OPTIONS
    "PNG_STATIC ON"
    "PNG_SHARED OFF")
else()
  set(LIBPNG_OPTIONS
    "PNG_STATIC OFF"
    "PNG_SHARED ON")
endif()

list(
  APPEND
  LIBPNG_OPTIONS
    "PNG_BUILD_ZLIB ON"
    "ZLIB_LIBRARIES ZLIB"
    "ZLIB_INCLUDE_DIRS ${ZLIB_SOURCE_DIR}\" \"${ZLIB_BINARY_DIR}")

CPMAddPackage(
  NAME PNG
  VERSION 1.6.39
  URL https://sourceforge.net/projects/libpng/files/libpng16/1.6.39/libpng-1.6.39.tar.xz
  URL_HASH SHA256=1f4696ce70b4ee5f85f1e1623dc1229b210029fa4b7aee573df3e2ba7b036937
  EXCLUDE_FROM_ALL TRUE
  DOWNLOAD_ONLY ${GRVL_ZEPHYR}
  OPTIONS
    "SKIP_INSTALL_EXPORT ON"
    "PNG_EXECUTABLES OFF"
    "PNG_TESTS OFF"
    ${LIBPNG_OPTIONS})

if(NOT GRVL_ZEPHYR)
  add_library(PNG INTERFACE)
  target_include_directories(JPEG INTERFACE ${PNG_SOURCE_DIR} ${PNG_BINARY_DIR})

  if(GRVL_STATIC OR NOT BUILD_SHARED_LIBS)
    target_link_libraries(PNG INTERFACE png_static)
  else()
    target_link_libraries(PNG INTERFACE png)
  endif()
endif()

# tinyxml2
CPMAddPackage(
  NAME tinyxml2
  VERSION 9.0.0
  GITHUB_REPOSITORY leethomason/tinyxml2
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE
  GIT_TAG 9.0.0
  EXCLUDE_FROM_ALL TRUE
  DOWNLOAD_ONLY ${GRVL_ZEPHYR}
  OPTIONS
    "tinyxml2_BUILD_TESTING OFF")

# Duktape
CPMAddPackage(
  NAME duktape
  VERSION 2.7.0
  URL https://github.com/svaarala/duktape/releases/download/v2.7.0/duktape-2.7.0.tar.xz
  URL_HASH SHA256=90f8d2fa8b5567c6899830ddef2c03f3c27960b11aca222fa17aa7ac613c2890
  DOWNLOAD_ONLY TRUE
  EXCLUDE_FROM_ALL TRUE)

if(NOT GRVL_ZEPHYR)
  add_library(duktape)

  file(
    GLOB_RECURSE duktape_sources "${duktape_SOURCE_DIR}/src/*.c"
                                 "${duktape_SOURCE_DIR}/src/*.h")

  target_sources(duktape PRIVATE "${duktape_sources}")
  target_include_directories(duktape PUBLIC "${duktape_SOURCE_DIR}/src")
endif()

