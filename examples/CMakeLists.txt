cmake_minimum_required(VERSION 3.5)
target_compile_definitions(libcaos PUBLIC CAOS_BUILD_EXAMPLES)

if(CAOS_USE_CROWCPP)
  add_subdirectory(crow_endpoint)
  add_subdirectory(crow_middleware)
endif()

add_subdirectory(stress)

