if(EXISTS ${CMAKE_SOURCE_DIR}/cmake/custom_code.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/custom_code.cmake)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE libcaos repoexception)

if(CAOS_USE_CROWCPP)
  target_link_libraries(${PROJECT_NAME} PRIVATE Crow::Crow)
endif()

include(${CMAKE_BINARY_DIR}/generated_queries/Query_Config.cmake)
