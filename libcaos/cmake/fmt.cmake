FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 12.0.0
)
FetchContent_MakeAvailable(fmt)

target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
