find_package(cxxopts REQUIRED)

if(NOT cxxopts_FOUND)
  message(FATAL_ERROR "cxxopts not found. Install with: sudo apt-get install libcxxopts-dev")
endif()
