add_library(grvl STATIC)

file(GLOB_RECURSE GRVL_SOURCE src/*.cpp src/*.c)
target_include_directories(grvl PUBLIC include/)

set(libjpeg_SOURCE_DIR ${libjpeg-turbo_SOURCE_DIR})

add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
add_subdirectory(${tinyxml2_SOURCE_DIR} ${tinyxml2_BINARY_DIR})
target_include_directories(grvl PUBLIC ${libjpeg_SOURCE_DIR})
target_include_directories(grvl PUBLIC ${libpng_SOURCE_DIR})
target_include_directories(grvl PUBLIC ${duktape_SOURCE_DIR}/src)

add_library(libjpeg STATIC)

file(GLOB LIBJPEG_SRC
  ${libjpeg_SOURCE_DIR}/*.c
)

list(FILTER LIBJPEG_SRC EXCLUDE REGEX "jmemdos.c|jmemmac.c")
target_sources(libjpeg PRIVATE ${LIBJPEG_SRC})

add_library(libpng STATIC)

if(NOT EXISTS ${libpng_SOURCE_DIR}/bin/lib/libpng15.a)
  ADD_CUSTOM_TARGET(
    libpng_build ALL
    COMMAND ./configure --prefix=${libpng_SOURCE_DIR}/bin 1>/dev/null && make install 1>/dev/null && rm *.o *.lo
    WORKING_DIRECTORY ${libpng_SOURCE_DIR}
    COMMENT "LIBPNG makefile target"
    BYPRODUCTS $${libpng_SOURCE_DIR}/bin/lib/libpng15.a
  )
  add_dependencies(libpng libpng_build)
endif()

file(GLOB PNGLIB_SRC
  ${libpng_SOURCE_DIR}/*.c
)

target_sources(libpng PRIVATE ${PNGLIB_SRC})

add_library(duktape STATIC)
file(GLOB DUKTAPE_SRC
  ${duktape_SOURCE_DIR}/src/*.c
)
target_sources(duktape PRIVATE ${DUKTAPE_SRC})

target_sources(grvl PRIVATE ${GRVL_SOURCE})
target_link_libraries(grvl PUBLIC zlib tinyxml2 libjpeg libpng duktape)
