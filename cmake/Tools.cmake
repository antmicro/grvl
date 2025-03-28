option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

if(ENABLE_CLANG_TIDY)
  find_program(
    CLANG_TIDY_EXE
    NAMES clang-tidy
    DOC "Path to clang-tidy executable")

  if(NOT CLANG_TIDY_EXE)
    message(STATUS "clang-tidy not found")
  else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(
      CMAKE_CXX_CLANG_TIDY
      "${CLANG_TIDY_EXE}"
      "--format-style=file"
      "--checks=-*,clang-diagnostic-*,clang-analyzer-*,-*,bugprone-*,performance-*,readability-*,-readability-identifier-length,-readability-braces-around-statements,-readability-implicit-bool-conversion,-bugprone-narrowing-conversions,-readability-function-cognitive-complexity,-bugprone-easily-swappable-parameters,-bugprone-parent-virtual-call,-performance-no-int-to-ptr"
      "--warnings-as-errors=*")
  endif()
endif()
