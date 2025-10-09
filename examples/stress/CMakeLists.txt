# examples/endpoint/CMakeLists.txt
cmake_minimum_required(VERSION 3.5)

add_executable(stress main.cpp)
set_target_properties(stress PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/bin/
)

# Configurazioni debug con sanitizer
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Sanitizer abilitati per build Debug")

  target_compile_options(stress PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fno-omit-frame-pointer
  )

  target_link_options(stress PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
  )
endif()

target_link_libraries(stress PUBLIC
    libcaos
    repoexception
)
