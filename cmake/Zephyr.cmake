if(NOT GRVL_ZEPHYR)
  return()
endif()

message("Enabling Zephyr support")

## Zlib ##
zephyr_library_named(ZLIB)

file(
  GLOB zlib_sources
  ${ZLIB_SOURCE_DIR}/*.c)

configure_file(${ZLIB_SOURCE_DIR}/zconf.h.in ${ZLIB_BINARY_DIR}/zconf.h)

zephyr_library_sources(${zlib_sources})
zephyr_library_compile_definitions(
  ZLIB_COMPAT
  WITH_GZFILEOP
  O_TRUNC=00001000)
zephyr_include_directories(
  ${ZLIB_BINARY_DIR}
  ${ZLIB_SOURCE_DIR})

## tinyxml2 ##
zephyr_library_named(tinyxml2)

file(
  GLOB tinyxml2_sources
  ${tinyxml2_SOURCE_DIR}/*.cpp)

list(FILTER tinyxml2_sources EXCLUDE REGEX "xmltest.cpp")

zephyr_library_sources(${tinyxml2_sources})
zephyr_include_directories(${tinyxml2_SOURCE_DIR})

## duktape ##
zephyr_library_named(duktape)

file(
  GLOB duktape_sources
  ${duktape_SOURCE_DIR}/src/*.c)

zephyr_library_sources(${duktape_sources})
zephyr_include_directories(${duktape_SOURCE_DIR}/src)
