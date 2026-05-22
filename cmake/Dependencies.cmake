
# We only support dynamically linked SDL
if (GRVL_LINUX_DESKTOP)
  find_package(SDL2 REQUIRED)
endif()

if (GRVL_LINUX_NATIVE)
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(LIBDRM REQUIRED IMPORTED_TARGET libdrm)
  get_target_property(lib_drm_includes PkgConfig::LIBDRM INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "LIBDRM includes: ${lib_drm_includes}")

  pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
  get_target_property(lib_xkbcommon_includes PkgConfig::LIBDRM INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "XKBCOMMON includes: ${lib_xkbcommon_includes}")

  pkg_check_modules(LIBINPUT REQUIRED IMPORTED_TARGET libinput)
  get_target_property(lib_libinput_includes PkgConfig::LIBINPUT INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "LIBINPUT includes: ${lib_libinput_includes}")

  pkg_check_modules(LIBUDEV REQUIRED IMPORTED_TARGET libudev)
  get_target_property(lib_libudev_includes PkgConfig::LIBUDEV INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "LIBUDEV includes: ${lib_libudev_includes}")
endif()

if(USE_SYSTEM_LIBRARIES)
  find_package(ZLIB REQUIRED)
  find_package(tinyxml2 REQUIRED)

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(DUKTAPE REQUIRED duktape)

  # Placeholder library
  add_library(libstb INTERFACE)

  add_library(ZLIB ALIAS ZLIB::ZLIB)
  return()
endif()

# Set minimum CMake Policy Version for dependencies
set(CMAKE_POLICY_VERSION_MINIMUM 3.16)

include(CPM)

CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG 28d546d5eb77d4585506a20480f4de2e706dff4c
)

file(CREATE_LINK "${stb_SOURCE_DIR}" "${stb_SOURCE_DIR}/stb" SYMBOLIC)

add_library(libstb INTERFACE)
target_include_directories(libstb INTERFACE "${stb_SOURCE_DIR}")

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
