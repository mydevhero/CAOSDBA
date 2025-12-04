# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM FMT LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for fmt library on the system...")

find_package(fmt QUIET)

if(fmt_FOUND)
  message(STATUS "Using system fmt library (version: ${fmt_VERSION})")

  # --------------------------------------------------------------------------------------------------
  # 2. LINK
  # --------------------------------------------------------------------------------------------------
  target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

else()
  message(FATAL_ERROR "FMT library not found. Install with: sudo apt-get install libfmt-dev")
endif()
