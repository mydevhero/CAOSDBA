# --------------------------------------------------------------------------------------------------
# 1. DETECT OR BUILD FMT
# --------------------------------------------------------------------------------------------------
set(FMT_PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/prebuilt/fmt")
set(FMT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmt")

if(EXISTS ${FMT_PREBUILT_DIR}/include/fmt/format.h)
  message(STATUS "Using pre-built fmt")

  message(STATUS "Using fmt via add_subdirectory for CMake compatibility")
  add_subdirectory(${FMT_SOURCE_DIR})

else()
  message(STATUS "Building fmt from submodule...")

  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_fmt.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE fmt_build_result
  )

  if(NOT fmt_build_result EQUAL 0)
    message(FATAL_ERROR "fmt build failed")
  endif()

  add_subdirectory(${FMT_SOURCE_DIR})
endif()

# --------------------------------------------------------------------------------------------------
# 2. LINK
# --------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# --------------------------------------------------------------------------------------------------
# 3. Build prebuilt
# --------------------------------------------------------------------------------------------------
if(NOT EXISTS ${FMT_PREBUILT_DIR}/include/fmt/format.h)
  message(STATUS "Creating fmt prebuilt libraries...")
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_fmt.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endif()
