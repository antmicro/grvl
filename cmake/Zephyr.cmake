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

## libjpeg ##
zephyr_library_named(JPEG)

set(simd_sources jsimd_none.c)
set(jpeg_sources ${simd_sources}
  jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c
  jcicc.c jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c
  jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c jdatadst.c
  jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c jdicc.c jdinput.c
  jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c jdpostct.c jdsample.c
  jdtrans.c jerror.c jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c
  jidctint.c jidctred.c jquant1.c jquant2.c jutils.c jmemmgr.c jmemnobs.c)

list(TRANSFORM jpeg_sources PREPEND ${JPEG_SOURCE_DIR}/)

set(JPEG_LIB_VERSION 62)
set(VERSION 2.0.2)
set(LIBJPEG_TURBO_VERSION_NUMBER 8)
set(C_ARITH_CODING_SUPPORTED 0)
set(D_ARITH_CODING_SUPPORTED 0)
set(MEM_SRCDST_SUPPORTED 1)
set(MEM_SRCDST_FUNCTIONS "global:  jpeg_mem_dest;  jpeg_mem_src;")
set(WITH_SIMD OFF)
set(BITS_IN_JSAMPLE 8)

set(INLINE inline)
set(THREAD_LOCAL Z_THREAD_LOCAL)
set(HAVE_BUILTIN_CTZL HAS_BUILTIN___builtin_ctzl)

if(CONFIG_64BIT)
  set(SIZE_T 8)
else()
  set(SIZE_T 4)
endif()

configure_file(${JPEG_SOURCE_DIR}/jconfig.h.in ${JPEG_BINARY_DIR}/jconfig.h)
configure_file(${JPEG_SOURCE_DIR}/jconfigint.h.in ${JPEG_BINARY_DIR}/jconfigint.h)

zephyr_library_sources(${jpeg_sources})
zephyr_library_compile_options(-Wno-implicit-function-declaration)
zephyr_include_directories(
  ${JPEG_BINARY_DIR}
  ${JPEG_SOURCE_DIR})

## libpng ##
zephyr_library_named(PNG)

file(
  GLOB png_sources
  ${PNG_SOURCE_DIR}/*.c)

list(FILTER png_sources EXCLUDE REGEX "pngtest.c|example.c")

set(PNG_STATIC ON)
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
set(PNG_EXECUTABLES OFF)
set(PNG_BUILD_ZLIB ON)
set(ZLIB_LIBRARIES ZLIB)
set(ZLIB_INCLUDE_DIRS "${ZLIB_SOURCE_DIR}\" \"${ZLIB_BINARY_DIR}")
set(SKIP_INSTALL_EXPORT ON)

add_subdirectory(${PNG_SOURCE_DIR} ${PNG_BINARY_DIR} EXCLUDE_FROM_ALL)

zephyr_library_sources(${png_sources})
zephyr_library_add_dependencies(genfiles)
zephyr_library_link_libraries(ZLIB)
zephyr_include_directories(
  ${PNG_BINARY_DIR}
  ${PNG_SOURCE_DIR})

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

