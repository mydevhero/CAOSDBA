# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM FMT LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for fmt library on the system...")

# Cerca il pacchetto fmt usando CMake
find_package(fmt QUIET)

if(fmt_FOUND)
  message(STATUS "Using system fmt library (version: ${fmt_VERSION})")

  # --------------------------------------------------------------------------------------------------
  # 2. LINK
  # --------------------------------------------------------------------------------------------------
  target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

else()
  # Se fmt non è trovato, fornisce istruzioni chiare
  message(FATAL_ERROR "\n"
    "===========================================================================\n"
    "FMT LIBRARY NOT FOUND\n"
    "===========================================================================\n"
    "The fmt library is required but was not found on your system.\n\n"
    "Please install fmt using one of the following methods:\n\n"
    "Ubuntu/Debian:\n"
    "    sudo apt-get install libfmt-dev\n\n"
    "Fedora/RHEL/CentOS:\n"
    "    sudo dnf install fmt-devel\n"
    "    # or for older versions:\n"
    "    sudo yum install fmt-devel\n\n"
    "Arch Linux:\n"
    "    sudo pacman -S fmt\n\n"
    "macOS (Homebrew):\n"
    "    brew install fmt\n\n"
    "Windows (vcpkg):\n"
    "    vcpkg install fmt\n\n"
    "From source:\n"
    "    git clone https://github.com/fmtlib/fmt.git\n"
    "    cd fmt && mkdir build && cd build\n"
    "    cmake .. && make && sudo make install\n\n"
    "Alternatively, you can set the FMT_ROOT environment variable to point\n"
    "to a custom installation directory.\n"
    "===========================================================================\n"
  )
endif()
