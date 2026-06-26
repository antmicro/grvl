cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(
  grvl
  LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 17)

include(StandardProjectSettings)
include(Tools)
include(Dependencies)
include(Zephyr)
include(Docs)

file(
  GLOB_RECURSE headers
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/grvl/*.h"
                    "${CMAKE_CURRENT_SOURCE_DIR}/include/grvl/platform/*.h")
file(
  GLOB_RECURSE sources
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
                    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(
  GLOB_RECURSE gbf_sources
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gbf/*.hpp"
                    "${CMAKE_CURRENT_SOURCE_DIR}/gbf/*.cpp")

# Platform specific code is added separately
list(FILTER sources EXCLUDE REGEX ".*/src/platform/.*")
list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/Posix.cpp"
                    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/Application.cpp"
                    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/LinuxGeneric.cpp")

if(GRVL_LINUX_NATIVE)
  list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/LinuxNative.cpp")
  target_compile_definitions(grvl_options INTERFACE GRVL_LINUX_NATIVE_SUPPORT)
  message(STATUS "GRVL Native Linux support ENABLED")
endif()

if(GRVL_LINUX_DESKTOP)
  list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/LinuxDesktop.cpp")
  target_compile_definitions(grvl_options INTERFACE GRVL_LINUX_DESKTOP_SUPPORT)
  message(STATUS "GRVL Desktop Linux support ENABLED")
endif()

if(GRVL_ZEPHYR)
  list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ZephyrApp.cpp")
  list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/Dma2d.cpp")
  target_compile_definitions(grvl_options INTERFACE GRVL_ZEPHYR_SUPPORT)
  message(STATUS "GRVL Zephyr support ENABLED")
endif()

add_library(grvl ${sources} ${headers})
set_target_properties(grvl PROPERTIES CXX_STANDARD 17
                                      CXX_EXTENSIONS OFF)

target_link_libraries(
  grvl
  PRIVATE grvl_options libstb
  PUBLIC ZLIB
         tinyxml2
         duktape

         $<$<BOOL:${GRVL_LINUX_DESKTOP}>:SDL2>
         $<$<BOOL:${GRVL_LINUX_NATIVE}>:PkgConfig::LIBDRM>
         $<$<BOOL:${GRVL_LINUX_NATIVE}>:PkgConfig::LIBUDEV>
         $<$<BOOL:${GRVL_LINUX_NATIVE}>:PkgConfig::LIBINPUT>
         $<$<BOOL:${GRVL_LINUX_NATIVE}>:PkgConfig::XKBCOMMON>
)

if (NOT GRVL_ZEPHYR)
  add_executable(gbf EXCLUDE_FROM_ALL ${gbf_sources})

  target_link_libraries(gbf PRIVATE grvl)

  if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(test)
  endif()
endif()

if(USE_SYSTEM_LIBRARIES AND NOT GRVL_ZEPHYR)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  set(
    INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/grvl
    CACHE PATH "Location of header files")
  set(
    LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}
    CACHE PATH "Location of library files")

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/grvlConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grvl
    PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR)

  install(
    TARGETS grvl
    EXPORT grvlTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_include_directories(
    grvl PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/grvl>
  )


  install(
    DIRECTORY include/grvl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/grvl
    FILES_MATCHING PATTERN "*.h")

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/grvlConfig.cmake
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grvl)

  install(
    EXPORT grvlTargets
    FILE grvlTargets.cmake
    NAMESPACE grvl::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grvl)
else()
  target_include_directories(
    grvl PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/grvl>
  )
endif()
