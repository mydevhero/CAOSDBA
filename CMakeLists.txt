cmake_minimum_required(VERSION 3.5)

include(cmake/policy.cmake)
include(cmake/compiler.cmake)

# Configure here libcaos
include(cmake/config.cmake)

# Custom application
if(EXISTS ${CMAKE_SOURCE_DIR}/cmake/app.cmake)
  project(my_app VERSION 0.0.1 LANGUAGES CXX)
  include(${CMAKE_SOURCE_DIR}/cmake/app.cmake)
  target_link_libraries(${PROJECT_NAME} PRIVATE libcaos repoexception)

# Standard CAOS project
else()
  project(CAOS VERSION 0.0.1 LANGUAGES CXX)
endif()


