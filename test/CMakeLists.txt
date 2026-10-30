Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
    button.cpp
    slider.cpp
    expressions.cpp
    text_input_cursor.cpp
    text_input_autofocus.cpp
    key_mappings.cpp
)

target_link_libraries(tests PRIVATE grvl Catch2::Catch2WithMain)

target_compile_definitions(tests PRIVATE
    "GRVL_TEST_FONT=\"${CMAKE_CURRENT_SOURCE_DIR}/../samples/empty/romfs/fonts/mona16.gbf\"")
