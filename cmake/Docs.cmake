if(BUILD_DOCS)
  find_package(Doxygen)
  find_package(Sphinx)

  set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/docs/src)
  set(SPHINX_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/build/sphinx)
  set(SPHINX_TARGETS html tex)

  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/doxydocs)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html)

  add_custom_command(
    OUTPUT ${DOXYGEN_INDEX_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_OUTPUT_DIRECTORY}
    COMMAND ${DOXYGEN_EXECUTABLE} docs/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY docs/Doxyfile
    COMMENT "Generating docs")
  add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

  add_custom_target(
    Sphinx_HTML ALL
    COMMAND ${SPHINX_EXECUTABLE} -b html ${SPHINX_SOURCE} ${SPHINX_BUILD}/html
            -Dbreathe_projects.grvl=${DOXYGEN_OUTPUT_DIRECTORY}/xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating HTML documentation with Sphinx")

  add_custom_target(
    Sphinx_PDF ALL
    COMMAND
      ${SPHINX_EXECUTABLE} -M latexpdf ${SPHINX_SOURCE} ${SPHINX_BUILD}/pdf
      -Dbreathe_projects.grvl=${DOXYGEN_OUTPUT_DIRECTORY}/xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating PDF documentation with Sphinx")

  add_dependencies(Sphinx_HTML Doxygen)
  add_dependencies(Sphinx_PDF Doxygen)
endif()
