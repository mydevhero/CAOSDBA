# examples/crow_middleware
cmake_minimum_required(VERSION 3.5)

add_executable(crow_middleware main.cpp)

set_target_properties(crow_middleware PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/bin/
)

if(CMAKE_BUILD_TYPE STREQUAL "debug")
  message(STATUS "Sanitizer on build Debug")

  target_compile_options(crow_middleware PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fno-omit-frame-pointer
  )

  target_link_options(crow_middleware PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
  )
endif()

target_link_libraries(crow_middleware PUBLIC
  libcaos
  repoexception
  Crow::Crow
)
