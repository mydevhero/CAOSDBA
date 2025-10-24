message(STATUS "Testing features enabled - CAOS_BUILD_TEST macro defined")

include(CTest)
enable_testing()

include(../cmake/CPM.cmake)

project(test LANGUAGES CXX)

CPMAddPackage(
  Catch2
  VERSION 3.9.0
  GITHUB_REPOSITORY catchorg/Catch2
  OPTIONS
  "CATCH_INSTALL_DOCS Off"
  "CATCH_INSTALL_EXTRAS Off"
  "CATCH_ENABLE_REPRODUCIBLE_BUILD Off"
)

message(STATUS "Configuring tests...")

add_executable(${PROJECT_NAME}
  test_main.cpp
  tests/terminal_options.hpp
  tests/log.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  spdlog::spdlog
  libcaos
  Catch2::Catch2WithMain
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  CAOS_USE_CACHE
  CAOS_CACHE_BACKEND
  CAOS_DB_BACKEND
  CAOS_USE_CROWCPP
  CAOS_BUILD_EXAMPLES
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/src/include
)

add_test(
  NAME ${PROJECT_NAME}
  COMMAND ${PROJECT_NAME}
)
