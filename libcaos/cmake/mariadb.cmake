target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_MARIADB)

# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM MARIADB C CONNECTOR
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for MariaDB C connector on the system...")

# Prova a trovare con CMake
find_package(MariaDB QUIET)
find_package(mariadb-connector-c QUIET)

# Se non trovato con CMake, prova con pkg-config
if(NOT MariaDB_FOUND AND NOT mariadb-connector-c_FOUND)
  pkg_check_modules(MARIADB_C QUIET libmariadb mariadb-client)
  if(MARIADB_C_FOUND)
    message(STATUS "Found MariaDB C connector via pkg-config")
    set(MariaDB_FOUND TRUE)
  endif()
endif()

# Se ancora non trovato, prova con find_library (approccio legacy)
if(NOT MariaDB_FOUND AND NOT mariadb-connector-c_FOUND)
  find_library(MARIADB_C_LIB
    NAMES mariadb mariadbclient mariadb3
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
  )
find_path(MARIADB_C_INCLUDE_DIR
  NAMES mysql.h
  PATHS /usr/include /usr/local/include
  PATH_SUFFIXES mysql mariadb
)

if(MARIADB_C_LIB AND MARIADB_C_INCLUDE_DIR)
  message(STATUS "Found MariaDB C connector via find_library")
  set(MariaDB_FOUND TRUE)
endif()
endif()

if(MariaDB_FOUND OR mariadb-connector-c_FOUND)
  message(STATUS "Using system MariaDB C connector")

  # Crea target importato per MariaDB C connector
  add_library(mariadb_c_connector INTERFACE IMPORTED)
  if(TARGET MariaDB::MariaDB)
    set_target_properties(mariadb_c_connector PROPERTIES
      INTERFACE_LINK_LIBRARIES "MariaDB::MariaDB"
    )
elseif(TARGET MariaDB::client)
  set_target_properties(mariadb_c_connector PROPERTIES
    INTERFACE_LINK_LIBRARIES "MariaDB::client"
  )
elseif(MARIADB_C_FOUND)
  set_target_properties(mariadb_c_connector PROPERTIES
    INTERFACE_LINK_LIBRARIES "${MARIADB_C_LINK_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_C_INCLUDE_DIRS}"
    INTERFACE_COMPILE_OPTIONS "${MARIADB_C_CFLAGS_OTHER}"
  )
else()
  set_target_properties(mariadb_c_connector PROPERTIES
    INTERFACE_LINK_LIBRARIES "${MARIADB_C_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_C_INCLUDE_DIR}"
  )
endif()

else()
  message(FATAL_ERROR "\n"
    "===========================================================================\n"
    "MARIADB C CONNECTOR NOT FOUND\n"
    "===========================================================================\n"
    "MariaDB C connector is required but was not found on your system.\n\n"
    "Please install MariaDB development packages using one of the following methods:\n\n"
    "Ubuntu/Debian:\n"
    "    sudo apt-get install libmariadb-dev libmariadb-client-lgpl-dev\n\n"
    "RedHat/Fedora/CentOS:\n"
    "    sudo dnf install mariadb-connector-c-devel\n"
    "    # or for older versions:\n"
    "    sudo yum install mariadb-connector-c-devel\n\n"
    "Arch Linux:\n"
    "    sudo pacman -S libmariadbclient\n\n"
    "macOS (Homebrew):\n"
    "    brew install mariadb-connector-c\n\n"
    "Windows (vcpkg):\n"
    "    vcpkg install mariadb-connector-c\n\n"
    "From source:\n"
    "    Download from: https://mariadb.com/downloads/connectors/connector-c\n\n"
    "After installation, run CMake again.\n"
    "===========================================================================\n"
  )
endif()

# --------------------------------------------------------------------------------------------------
# 2. FIND SYSTEM MARIADB CONNECTOR/C++
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for MariaDB Connector/C++ on the system...")

# Prova a trovare con CMake
find_package(mariadb-connector-cpp QUIET)

# Se non trovato con CMake, prova con pkg-config
if(NOT mariadb-connector-cpp_FOUND)
  pkg_check_modules(MARIADB_CPP QUIET mariadb-connector-cpp libmariadbcpp)
  if(MARIADB_CPP_FOUND)
    message(STATUS "Found MariaDB Connector/C++ via pkg-config")
    set(mariadb-connector-cpp_FOUND TRUE)
  endif()
endif()

# Se ancora non trovato, prova con find_library
if(NOT mariadb-connector-cpp_FOUND)
  find_library(MARIADB_CPP_LIB
    NAMES mariadbcpp mariadbpp mariadb-connector-cpp
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
  )
find_path(MARIADB_CPP_INCLUDE_DIR
  NAMES mariadb_connector.hpp mariadb/connector.hpp
  PATHS /usr/include /usr/local/include
  PATH_SUFFIXES mariadb mysql-cpp-connector
)

if(MARIADB_CPP_LIB AND MARIADB_CPP_INCLUDE_DIR)
  message(STATUS "Found MariaDB Connector/C++ via find_library")
  set(mariadb-connector-cpp_FOUND TRUE)
endif()
endif()

if(mariadb-connector-cpp_FOUND)
  message(STATUS "Using system MariaDB Connector/C++")

  # Crea target importato per MariaDB Connector/C++
  add_library(mariadb_cpp_connector INTERFACE IMPORTED)
  if(TARGET MariaDB::connector-cpp)
    set_target_properties(mariadb_cpp_connector PROPERTIES
      INTERFACE_LINK_LIBRARIES "MariaDB::connector-cpp"
    )
elseif(MARIADB_CPP_FOUND)
  set_target_properties(mariadb_cpp_connector PROPERTIES
    INTERFACE_LINK_LIBRARIES "${MARIADB_CPP_LINK_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_CPP_INCLUDE_DIRS}"
    INTERFACE_COMPILE_OPTIONS "${MARIADB_CPP_CFLAGS_OTHER}"
  )
else()
  set_target_properties(mariadb_cpp_connector PROPERTIES
    INTERFACE_LINK_LIBRARIES "${MARIADB_CPP_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_CPP_INCLUDE_DIR}"
  )
endif()

# Assicurati che il Connector/C++ linki con il C connector
set_target_properties(mariadb_cpp_connector PROPERTIES
  INTERFACE_LINK_LIBRARIES "mariadb_c_connector"
)

else()
  message(FATAL_ERROR "\n"
    "===========================================================================\n"
    "MARIADB CONNECTOR/C++ NOT FOUND\n"
    "===========================================================================\n"
    "MariaDB Connector/C++ is required but was not found on your system.\n\n"
    "Please install MariaDB Connector/C++ using one of the following methods:\n\n"
    "Ubuntu/Debian:\n"
    "    sudo apt-get install libmariadbcpp-dev\n\n"
    "RedHat/Fedora/CentOS:\n"
    "    sudo dnf install mariadb-connector-cpp-devel\n"
    "    # or for EPEL:\n"
    "    sudo dnf install epel-release && sudo dnf install mariadb-connector-cpp-devel\n\n"
    "Arch Linux (AUR):\n"
    "    yay -S mariadb-connector-c++\n\n"
    "macOS (Homebrew):\n"
    "    brew install mariadb-connector-c++\n\n"
    "Windows (vcpkg):\n"
    "    vcpkg install mariadb-connector-cpp\n\n"
    "From source:\n"
    "    Download from: https://mariadb.com/downloads/connectors/connector-cpp\n"
    "    or\n"
    "    git clone https://github.com/mariadb-corporation/mariadb-connector-cpp.git\n"
    "    cd mariadb-connector-cpp && mkdir build && cd build\n"
    "    cmake .. && make && sudo make install\n\n"
    "Note: MariaDB Connector/C++ requires MariaDB C connector to be installed first.\n"
    "After installation, run CMake again.\n"
    "===========================================================================\n"
  )
endif()

# --------------------------------------------------------------------------------------------------
# 3. LINK ALL
# --------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE mariadb_cpp_connector mariadb_c_connector)

message(STATUS "MariaDB setup complete using system libraries")
