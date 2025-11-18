# --------------------------------------------------------------------------------------------------
# FIND SYSTEM SPDLOG LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for spdlog library on the system...")

# Prova a trovare con CMake (spdlog fornisce file di configurazione CMake)
find_package(spdlog QUIET)

if(spdlog_FOUND)
  message(STATUS "Using system spdlog library (version: ${spdlog_VERSION})")

  # Usa il target fornito da spdlog
  target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

  message(STATUS "spdlog configured successfully via system library")

else()
  message(FATAL_ERROR "\n"
    "===========================================================================\n"
    "SPDLOG LIBRARY NOT FOUND\n"
    "===========================================================================\n"
    "spdlog library is required but was not found on your system.\n\n"
    "Please install spdlog using one of the following methods:\n\n"
    "Ubuntu/Debian:\n"
    "    sudo apt-get install libspdlog-dev\n\n"
    "===========================================================================\n"
  )
endif()
