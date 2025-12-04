# --------------------------------------------------------------------------------------------------
# FIND SYSTEM SPDLOG LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for spdlog library on the system...")

find_package(spdlog QUIET)

if(spdlog_FOUND)
  message(STATUS "Using system spdlog library (version: ${spdlog_VERSION})")

  target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

  message(STATUS "spdlog configured successfully via system library")

else()
  message(FATAL_ERROR "SPDLOG library not found. Install with: sudo apt-get install libspdlog-dev")
endif()
