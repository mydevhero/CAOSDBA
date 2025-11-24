cmake_minimum_required(VERSION 3.5)

include(cmake/policy.cmake)
include(cmake/compiler.cmake)

# Configure here libcaos
include(cmake/config.cmake)

# Custom application
if(EXISTS ${CMAKE_SOURCE_DIR}/cmake/app.cmake)
  # Project Name
  project(my_app VERSION 0.0.1 LANGUAGES CXX)

  # Application
  include(${CMAKE_SOURCE_DIR}/cmake/app.cmake)

  # Add libcaos
  include(${CMAKE_SOURCE_DIR}/cmake/libcaos.cmake)

# Standard CAOS project
else()
  project(CAOS VERSION 0.0.1 LANGUAGES CXX)
endif()


