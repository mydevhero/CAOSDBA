find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
endif()

if(GIT_TAG)
  string(REGEX REPLACE "^v" "" PROJECT_VERSION "${GIT_TAG}")
else()
  set(PROJECT_VERSION "0.1.0")
endif()

message(STATUS "Project version: ${PROJECT_VERSION}")
