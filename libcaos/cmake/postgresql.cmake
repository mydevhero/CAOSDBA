target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_POSTGRESQL)

# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM POSTGRESQL LIBPQ LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for PostgreSQL libpq library on the system...")

# Prova a trovare con CMake
find_package(PostgreSQL QUIET)

# Se non trovato con CMake, prova con pkg-config
if(NOT PostgreSQL_FOUND)
    pkg_check_modules(LIBPQ QUIET libpq)
    if(LIBPQ_FOUND)
        message(STATUS "Found libpq via pkg-config")
        set(PostgreSQL_FOUND TRUE)
        set(PostgreSQL_INCLUDE_DIRS ${LIBPQ_INCLUDE_DIRS})
        set(PostgreSQL_LIBRARY_DIRS ${LIBPQ_LIBRARY_DIRS})
    endif()
endif()

if(PostgreSQL_FOUND)
    message(STATUS "Using system PostgreSQL libpq library")

    # Crea target importato per libpq
    add_library(libpq INTERFACE IMPORTED)
    if(TARGET PostgreSQL::PostgreSQL)
        # Se CMake fornisce un target importato
        set_target_properties(libpq PROPERTIES
            INTERFACE_LINK_LIBRARIES "PostgreSQL::PostgreSQL"
        )
    elseif(LIBPQ_FOUND)
        # Se trovato via pkg-config
        set_target_properties(libpq PROPERTIES
            INTERFACE_LINK_LIBRARIES "${LIBPQ_LINK_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBPQ_INCLUDE_DIRS}"
            INTERFACE_COMPILE_OPTIONS "${LIBPQ_CFLAGS_OTHER}"
        )
    else()
        # Fallback generico
        set_target_properties(libpq PROPERTIES
            INTERFACE_LINK_LIBRARIES "pq"
        )
    endif()

else()
    message(FATAL_ERROR "\n"
        "===========================================================================\n"
        "POSTGRESQL LIBPQ LIBRARY NOT FOUND\n"
        "===========================================================================\n"
        "PostgreSQL libpq library is required but was not found on your system.\n\n"
        "Please install PostgreSQL development packages using one of the following methods:\n\n"
        "Ubuntu/Debian:\n"
        "    sudo apt-get install libpq-dev postgresql-server-dev-all\n\n"
        "RedHat/Fedora/CentOS:\n"
        "    sudo dnf install libpq-devel postgresql-devel\n"
        "    # or for older versions:\n"
        "    sudo yum install libpq-devel postgresql-devel\n\n"
        "Arch Linux:\n"
        "    sudo pacman -S postgresql-libs\n\n"
        "macOS (Homebrew):\n"
        "    brew install postgresql\n\n"
        "Windows (vcpkg):\n"
        "    vcpkg install libpq\n\n"
        "From source:\n"
        "    Download from: https://www.postgresql.org/download/\n\n"
        "After installation, run CMake again.\n"
        "===========================================================================\n"
    )
endif()

# --------------------------------------------------------------------------------------------------
# 2. FIND SYSTEM LIBPQXX LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for libpqxx library on the system...")

# Prova a trovare con CMake
find_package(libpqxx QUIET)

# Se non trovato con CMake, prova con pkg-config
if(NOT libpqxx_FOUND)
    pkg_check_modules(LIBPQXX QUIET libpqxx)
    if(LIBPQXX_FOUND)
        message(STATUS "Found libpqxx via pkg-config (version: ${LIBPQXX_VERSION})")
        set(libpqxx_FOUND TRUE)
    endif()
endif()

if(libpqxx_FOUND)
    message(STATUS "Using system libpqxx library")

    # Crea target importato per libpqxx
    add_library(pqxx INTERFACE IMPORTED)
    if(TARGET libpqxx::pqxx)
        # Se CMake fornisce un target importato
        set_target_properties(pqxx PROPERTIES
            INTERFACE_LINK_LIBRARIES "libpqxx::pqxx"
        )
    elseif(LIBPQXX_FOUND)
        # Se trovato via pkg-config
        set_target_properties(pqxx PROPERTIES
            INTERFACE_LINK_LIBRARIES "${LIBPQXX_LINK_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBPQXX_INCLUDE_DIRS}"
            INTERFACE_COMPILE_OPTIONS "${LIBPQXX_CFLAGS_OTHER}"
        )
    else()
        # Fallback generico
        set_target_properties(pqxx PROPERTIES
            INTERFACE_LINK_LIBRARIES "pqxx"
        )
    endif()

    # Assicurati che libpqxx linki con libpq
    set_target_properties(pqxx PROPERTIES
        INTERFACE_LINK_LIBRARIES "libpq"
    )

else()
    message(FATAL_ERROR "\n"
        "===========================================================================\n"
        "LIBPQXX LIBRARY NOT FOUND\n"
        "===========================================================================\n"
        "libpqxx library is required but was not found on your system.\n\n"
        "Please install libpqxx using one of the following methods:\n\n"
        "Ubuntu/Debian:\n"
        "    sudo apt-get install libpqxx-dev\n\n"
        "RedHat/Fedora/CentOS:\n"
        "    sudo dnf install libpqxx-devel\n"
        "    # or for EPEL:\n"
        "    sudo dnf install epel-release && sudo dnf install libpqxx-devel\n\n"
        "Arch Linux:\n"
        "    sudo pacman -S libpqxx\n\n"
        "macOS (Homebrew):\n"
        "    brew install libpqxx\n\n"
        "Windows (vcpkg):\n"
        "    vcpkg install libpqxx\n\n"
        "From source:\n"
        "    git clone https://github.com/jtv/libpqxx.git\n"
        "    cd libpqxx && mkdir build && cd build\n"
        "    cmake .. && make && sudo make install\n\n"
        "Note: libpqxx requires PostgreSQL libpq to be installed first.\n"
        "After installation, run CMake again.\n"
        "===========================================================================\n"
    )
endif()

# --------------------------------------------------------------------------------------------------
# 3. LINK ALL
# --------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE pqxx libpq)

message(STATUS "PostgreSQL+libpqxx setup complete using system libraries")
