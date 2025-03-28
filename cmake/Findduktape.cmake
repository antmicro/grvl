# ~~~
# Locate duktape
#
# This module defines the following IMPORTED target:
#  duktape::duktape - the duktape library, if found
#
# This module defines the following variables:
#  DUKTAPE_FOUND - if true, duktape library found
#  DUKTAPE_INCLUDE_DIRS - where to find duktape.h
#  DUKTAPE_LIBRARIES - the libraries needed to use duktape
#  DUKTAPE_VERSION - the version of the duktape library found
# ~~~

find_package(PkgConfig QUIET)
pkg_check_modules(PC_DUK QUIET duktape libduktape)

find_path(
  DUKTAPE_INCLUDE_DIR duktape.h
  HINTS ${PC_DUK_INCLUDEDIR} ${PC_DUK_INCLUDE_DIRS}
  PATH_SUFFIXES duktape)

find_library(
  DUKTAPE_LIBRARY
  NAMES duktape libduktape
  HINTS ${PC_DUK_LIBDIR} ${PC_DUK_LIBRARY_DIRS})

if(DUKTAPE_INCLUDE_DIR)
  file(
    STRINGS "${DUKTAPE_INCLUDE_DIR}/duktape.h"
    duktape_version_line
    REGEX "^#define[ \t]+DUK_VERSION[ \t]+[0-9]+[L]?")

  if(duktape_version_line)
    string(
      REGEX
      REPLACE
        ".*[ \t]+([0-9]+)[L]?"
        "\\1"
      duktape_version
      "${duktape_version_line}")

    math(EXPR duktape_version_major "${duktape_version} / 10000")
    math(EXPR duktape_version_minor "(${duktape_version} % 10000) / 100")
    math(EXPR duktape_version_patch "${duktape_version} % 100")

    set(DUKTAPE_VERSION
      "${duktape_version_major}.${duktape_version_minor}.${duktape_version_patch}")
  endif()

  unset(duktape_version)
  unset(duktape_version_line)
  unset(duktape_version_major)
  unset(duktape_version_minor)
  unset(duktape_version_patch)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  duktape
  REQUIRED_VARS
    DUKTAPE_LIBRARY
    DUKTAPE_INCLUDE_DIR
  VERSION_VAR
    DUKTAPE_VERSION)

if(DUKTAPE_FOUND)
  set(DUKTAPE_LIBRARIES ${DUKTAPE_LIBRARY})
  set(DUKTAPE_INCLUDE_DIRS ${DUKTAPE_INCLUDE_DIR})
  add_library(duktape::duktape INTERFACE IMPORTED)
  set_target_properties(
    duktape::duktape
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${DUKTAPE_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${DUKTAPE_LIBRARIES}")
  if(NOT TARGET duktape)
    add_library(duktape ALIAS duktape::duktape)
  endif()
endif()

mark_as_advanced(DUKTAPE_INCLUDE_DIR DUKTAPE_LIBRARY)
