cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(
  grvl
  LANGUAGES C CXX)

include(StandardProjectSettings)
include(Tools)
include(Dependencies)
include(Zephyr)
include(Docs)

file(
  GLOB_RECURSE headers
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(
  GLOB_RECURSE sources
  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
                    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(grvl ${sources} ${headers})
set_target_properties(grvl PROPERTIES CXX_STANDARD 17
                                      CXX_EXTENSIONS OFF)

target_link_libraries(
  grvl
  PRIVATE grvl_options
  PUBLIC ZLIB
         JPEG
         PNG
         tinyxml2
         duktape)


target_include_directories(
  grvl PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include/grvl>)

if(USE_SYSTEM_LIBRARIES)
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

  install(
    DIRECTORY include/
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
endif()
