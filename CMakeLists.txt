cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0066 NEW)
cmake_policy(SET CMP0102 NEW)
cmake_policy(SET CMP0126 NEW)
cmake_policy(SET CMP0128 NEW)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# set(CMAKE_C_COMPILER "clang")
# set(CMAKE_CXX_COMPILER "clang++")

option(CAOS_USE_CCACHE "Uses CCache" ON)
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

option(CAOS_BUILD_EXAMPLES "Build examples" ON)

option(CAOS_USE_CACHE "CAOS using Cache" ON)
if(CAOS_USE_CACHE)
  set(CAOS_CACHE_BACKEND "REDIS" CACHE STRING "Cache backend to use for CAOS." )

  set_property(CACHE CAOS_CACHE_BACKEND PROPERTY STRINGS
    "REDIS"
  )
endif()

set(CAOS_DB_BACKEND "POSTGRESQL" CACHE STRING "Database backend to use for CAOS." )
set_property(CACHE CAOS_DB_BACKEND PROPERTY STRINGS
  "POSTGRESQL"
  "MYSQL"
  "MARIADB"
)

option(CAOS_USE_CROWCPP "CAOS use Crow" ON)

add_subdirectory(libcaos)

add_library(repoexception INTERFACE)
target_include_directories(repoexception INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libcaos/Middleware/Repository)
target_link_libraries(repoexception INTERFACE libcaos)

if(CAOS_ENABLE_COVERAGE)
  target_compile_definitions(libcaos PUBLIC CAOS_ENABLE_TESTS)
  target_compile_options(libcaos PRIVATE -fprofile-instr-generate -fcoverage-mapping)
  target_link_options(libcaos PRIVATE -fprofile-instr-generate -fcoverage-mapping)
endif()

if(CAOS_ENABLE_TESTS)
  target_compile_definitions(libcaos PUBLIC CAOS_ENABLE_TESTS)
  message(STATUS "Testing features enabled - CAOS_ENABLE_TESTS macro defined")

  include(CTest)
  enable_testing()

  include(libcaos/cmake/CPM.cmake)

  CPMAddPackage(
    Catch2
    VERSION 3.9.0
    GITHUB_REPOSITORY catchorg/Catch2
    OPTIONS
    "CATCH_INSTALL_DOCS Off"
    "CATCH_INSTALL_EXTRAS Off"
    "CATCH_ENABLE_REPRODUCIBLE_BUILD Off"
  )

  add_subdirectory(test)
endif()

if(CAOS_BUILD_EXAMPLES)
  add_subdirectory(examples)
  target_compile_definitions(libcaos PUBLIC CAOS_BUILD_EXAMPLES)
endif()

