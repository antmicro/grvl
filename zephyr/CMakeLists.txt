if(CONFIG_GRVL)
  set(GRVL_ZEPHYR ON)
  add_subdirectory(${ZEPHYR_GRVL_MODULE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/build)

  zephyr_include_directories(${ZEPHYR_GRVL_MODULE_DIR}/include)
  zephyr_append_cmake_library(grvl)
endif()
