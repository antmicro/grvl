Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
    button.cpp
)

target_link_libraries(tests PRIVATE grvl Catch2::Catch2WithMain)
