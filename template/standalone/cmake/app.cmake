# Main application executable
add_executable(${PROJECT_NAME} src/main.cpp)

include("${CMAKE_SOURCE_DIR}/cmake/app_debug.cmake")

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  libcaos
  repoexception
)
