cmake_minimum_required(VERSION 3.5)

include(${CMAKE_BINARY_DIR}/generated_queries/Query_Config.cmake)

target_compile_definitions(libcaos PUBLIC CAOS_BUILD_EXAMPLES)

if(QUERY_EXISTS_IQuery_Example_echoString)
  if(CAOS_BUILD_CROWCPP_EXAMPLE)
    target_compile_definitions(libcaos PUBLIC CAOS_BUILD_CROWCPP_EXAMPLE)
    add_subdirectory(crow_endpoint)
    add_subdirectory(crow_middleware)
  endif()

  if(CAOS_BUILD_STRESS_EXAMPLE)
    add_subdirectory(stress)
  endif()
endif()
