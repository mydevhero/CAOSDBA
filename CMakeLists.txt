cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0066 NEW)
cmake_policy(SET CMP0102 NEW)
cmake_policy(SET CMP0126 NEW)
cmake_policy(SET CMP0128 NEW)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# set(CMAKE_C_COMPILER "clang")
# set(CMAKE_CXX_COMPILER "clang++")

option(CAOS_USE_CCACHE "Uses CCache" OFF)
if(CAOS_USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      message(STATUS "ccache enabled")
  else()
      message(STATUS "ccache not found, install with: sudo apt install ccache")
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project Name
project(CAOS VERSION 0.0.1 LANGUAGES CXX)

# Select CAOS_ENV ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "debug" CACHE STRING "CAOS Environment." )
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "debug"
    "test"
    "release"
  )
endif()
# Select CAOS_ENV ----------------------------------------------------------------------------------

set(CAOS_USE_CACHE           "CAOS using Cache"          ON)
set(CAOS_BUILD_EXAMPLES      "CAOS build examples"       ON)
set(CAOS_USE_CROWCPP         "CAOS use Crow"             ON)
set(CAOS_BUILD_EXTENSIONS    "CAOS build bindings extension"  ON)
# option(CAOS_CHECK_COVERAGE  "CAOS check coverage" OFF)



# EXTENSIONS +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_EXTENSIONS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(CAOS_BUILD_PHP_EXTENSION    "CAOS build php extension"      ON)
  set(CAOS_BUILD_NODE_EXTENSION   "CAOS build node.js extension"  ON)
  set(CAOS_BUILD_PYTHON_EXTENSION "CAOS build node.js extension"  ON)
  add_subdirectory(extensions)
endif()
# EXTENSIONS ---------------------------------------------------------------------------------------



# Select CAOS_DB_BACKEND +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
set(CAOS_DB_BACKEND "POSTGRESQL" CACHE STRING "Database backend to use for CAOS." )
set_property(CACHE CAOS_DB_BACKEND PROPERTY STRINGS
  "POSTGRESQL"
  "MYSQL"
  "MARIADB"
)
# Select CAOS_DB_BACKEND ---------------------------------------------------------------------------



# Select CAOS_USE_CACHE ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_USE_CACHE)
  set(CAOS_CACHE_BACKEND "REDIS" CACHE STRING "Cache backend to use for CAOS." )

  set_property(CACHE CAOS_CACHE_BACKEND PROPERTY STRINGS
    "REDIS"
  )
endif()
# Select CAOS_USE_CACHE ----------------------------------------------------------------------------


# Add libcaos ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
add_subdirectory(libcaos)
add_library(repoexception INTERFACE)
target_include_directories(repoexception INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libcaos/Middleware/Repository)
target_link_libraries(repoexception INTERFACE libcaos)
# Add libcaos --------------------------------------------------------------------------------------



# Option CAOS_BUILD_EXAMPLES +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
# Option CAOS_BUILD_EXAMPLES -----------------------------------------------------------------------
