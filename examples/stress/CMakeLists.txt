# examples/endpoint/CMakeLists.txt
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(stress LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/bin/
)

# Configurazioni debug con sanitizer
if(CMAKE_BUILD_TYPE STREQUAL "debug")
  message(STATUS "Sanitizer abilitati per build Debug")

  target_compile_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fno-omit-frame-pointer
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
  )
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
  libcaos
  repoexception
)
