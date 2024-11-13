
file(GLOB_RECURSE GRVL_SOURCE src/*.cpp src/*.c)

zephyr_library_named(grvl)
target_include_directories(grvl PUBLIC include/)

set(libjpeg_SOURCE_DIR ${libjpeg-turbo_SOURCE_DIR})

zephyr_library_sources(${GRVL_SOURCE})
zephyr_include_directories(${zlib_SOURCE_DIR})
zephyr_include_directories(${tinyxml2_SOURCE_DIR})
zephyr_include_directories(${libpng_SOURCE_DIR})
zephyr_include_directories(${libjpeg_SOURCE_DIR})
zephyr_include_directories(${duktape_SOURCE_DIR}/src)

zephyr_library_named(tinyxml2)
zephyr_library_sources(${tinyxml2_SOURCE_DIR}/tinyxml2.cpp)

zephyr_library_named(libjpeg)
zephyr_library_compile_options(-DVMS=1 -DDONT_USE_B_MODE=1)  # workaround for ansi2knr.c to use void free()

if(NOT EXISTS ${libjpeg_SOURCE_DIR}/jconfig.h)
  ADD_CUSTOM_TARGET(
    libjpeg_conf ALL
    COMMAND ./configure 1>/dev/null
    WORKING_DIRECTORY ${libjpeg_SOURCE_DIR}
    COMMENT "LIBJPEG autoconf"
    BYPRODUCTS ${libjpeg_SOURCE_DIR}/jconfig.h
  )
  add_dependencies(libjpeg libjpeg_conf)
endif()

file(GLOB LIBJPEG_SRC
  ${libjpeg_SOURCE_DIR}/*.c
)

list(FILTER LIBJPEG_SRC EXCLUDE REGEX "jmemdos.c|jmemmac.c")

zephyr_library_sources(${LIBJPEG_SRC})

zephyr_library_named(zlib)
zephyr_library_compile_options(-DO_TRUNC=00001000)  # workaround for it to work with picolibc

file(GLOB ZLIB_SRC
  ${zlib_SOURCE_DIR}/*.c
)

zephyr_library_sources(${ZLIB_SRC})

zephyr_library_named(libpng)
if(NOT EXISTS ${libpng_SOURCE_DIR}/bin/lib/libpng15.a)
  ADD_CUSTOM_TARGET(
    libpng_build ALL
    COMMAND ./configure --prefix=${libpng_SOURCE_DIR}/bin 1>/dev/null && make install 1>/dev/null && rm *.o *.lo
    WORKING_DIRECTORY ${libpng_SOURCE_DIR}
    COMMENT "LIBPNG makefile target"
    BYPRODUCTS $${libpng_SOURCE_DIR}/bin/lib/libpng15.a
  )
  add_dependencies(libpng libpng_build)
  #add_dependencies(libpng zlib)
endif()

file(GLOB PNGLIB_SRC
  ${libpng_SOURCE_DIR}/*.c
)

message("pnglib src" ${PNGLIB_SRC})

zephyr_library_sources(${PNGLIB_SRC})

zephyr_library_named(duktape)
file(GLOB DUKTAPE_SRC
  ${duktape_SOURCE_DIR}/src/*.c
)
zephyr_library_sources(${DUKTAPE_SRC})

target_link_libraries(grvl PUBLIC zlib tinyxml2 libjpeg libpng duktape)
