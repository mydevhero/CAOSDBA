# examples/crow_endpoint
cmake_minimum_required(VERSION 3.5)

add_executable(crow_endpoint main.cpp)

set_target_properties(crow_endpoint PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/bin/
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Sanitizer on build Debug")

  target_compile_options(crow_endpoint PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fno-omit-frame-pointer
  )

  target_link_options(crow_endpoint PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
  )
endif()

target_link_libraries(crow_endpoint PUBLIC
  libcaos
  repoexception
  Crow::Crow
)
