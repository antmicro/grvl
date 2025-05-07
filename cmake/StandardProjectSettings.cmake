# grvl build options
option(GRVL_STATIC "Build static library" ON)
option(GRVL_SHARED "Build shared library" OFF)

option(GRVL_ZEPHYR "Enable ZephyrRTOS support" OFF)

option(BUILD_DOCS "Build documentation" OFF)
option(USE_SYSTEM_LIBRARIES "Use system provided libraries" OFF)

# Set default build type if none provided
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES AND NOT GRVL_ZEPHYR)
  message(
    STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui, ccmake
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS
             "Debug"
             "Release"
             "MinSizeRel"
             "RelWithDebInfo")
endif()

# Auto-create a .gitignore in the specified "build" directory.
if(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  file(
    GENERATE
    OUTPUT .gitignore
    CONTENT "*")
endif()

# Generate compile_commands.json for clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Interface target for compile options
add_library(grvl_options INTERFACE)

if(NOT GRVL_ZEPHYR)
  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(grvl_options INTERFACE -fcolor-diagnostics)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(grvl_options INTERFACE -fdiagnostics-color=always)
  endif()
endif()

if(GRVL_STATIC AND GRVL_SHARED)
  message(FATAL_ERROR "Building both shared and static library at the same time is not supported.")
endif()

if(GRVL_STATIC)
  set(BUILD_SHARED_LIBS OFF)
elseif(GRVL_SHARED)
  set(BUILD_SHARED_LIBS ON)
endif()

