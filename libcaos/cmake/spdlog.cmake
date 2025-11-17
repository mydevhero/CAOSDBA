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
    "RedHat/Fedora/CentOS:\n"
    "    sudo dnf install spdlog-devel\n"
    "    # or for EPEL:\n"
    "    sudo dnf install epel-release && sudo dnf install spdlog-devel\n\n"
    "Arch Linux:\n"
    "    sudo pacman -S spdlog\n\n"
    "macOS (Homebrew):\n"
    "    brew install spdlog\n\n"
    "Windows (vcpkg):\n"
    "    vcpkg install spdlog\n\n"
    "From source:\n"
    "    git clone https://github.com/gabime/spdlog.git\n"
    "    cd spdlog && mkdir build && cd build\n"
    "    cmake .. && make && sudo make install\n\n"
    "Note: spdlog requires a C++11 compatible compiler and fmt library.\n"
    "After installation, run CMake again.\n"
    "===========================================================================\n"
  )
endif()
