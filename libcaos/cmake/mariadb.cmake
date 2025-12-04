target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_MARIADB)

# --------------------------------------------------------------------------------------------------
# 1. DETECT OR BUILD MARIADB CONNECTOR
# --------------------------------------------------------------------------------------------------
set(MARIADB_PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/prebuilt/")
set(MARIADB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/mariadb-connector-cpp")

# Check if prebuilt libraries exist
if(EXISTS ${MARIADB_PREBUILT_DIR}/mariadb-connector/lib/mariadb/libmariadbcpp.a OR
   EXISTS ${MARIADB_PREBUILT_DIR}/mariadb-connector/lib/libmariadbcpp.a OR
   EXISTS ${MARIADB_PREBUILT_DIR}/mariadb-connector/lib/mariadb/libmariadb.a OR
   EXISTS ${MARIADB_PREBUILT_DIR}/mariadb-connector/lib/libmariadb.a)
  message(STATUS "Using pre-built MariaDB connector libraries")
  set(MARIADB_INSTALL_DIR ${MARIADB_PREBUILT_DIR}/mariadb-connector)
else()
  message(STATUS "Building MariaDB connector from submodule...")

  # Check if submodule exists
  if(NOT EXISTS ${MARIADB_SOURCE_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "MariaDB connector submodule not found. Initialize with: git submodule update --init --recursive")
  endif()

  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_mariadb_connector.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE mariadb_build_result
  )

  if(NOT mariadb_build_result EQUAL 0)
    message(FATAL_ERROR "MariaDB connector build failed with exit code: ${mariadb_build_result}")
  endif()

  set(MARIADB_INSTALL_DIR "${CMAKE_BINARY_DIR}/mariadb-connector-install")
endif()

# --------------------------------------------------------------------------------------------------
# 2. FIND LIBRARIES AND HEADERS IN CORRECT LOCATIONS
# --------------------------------------------------------------------------------------------------

# Try to find libmariadbcpp.a
if(EXISTS ${MARIADB_INSTALL_DIR}/lib/mariadb/libmariadbcpp.a)
  set(MARIADB_CPP_LIB ${MARIADB_INSTALL_DIR}/lib/mariadb/libmariadbcpp.a)
elseif(EXISTS ${MARIADB_INSTALL_DIR}/lib/libmariadbcpp.a)
  set(MARIADB_CPP_LIB ${MARIADB_INSTALL_DIR}/lib/libmariadbcpp.a)
else()
  message(FATAL_ERROR "MariaDB C++ connector library not found in: ${MARIADB_INSTALL_DIR}")
endif()

# Try to find libmariadb.a (C connector)
if(EXISTS ${MARIADB_INSTALL_DIR}/lib/mariadb/libmariadb.a)
  set(MARIADB_C_LIB ${MARIADB_INSTALL_DIR}/lib/mariadb/libmariadb.a)
elseif(EXISTS ${MARIADB_INSTALL_DIR}/lib/libmariadb.a)
  set(MARIADB_C_LIB ${MARIADB_INSTALL_DIR}/lib/libmariadb.a)
else()
  # Fallback: try to find system MariaDB C library
  find_library(SYSTEM_MARIADB_C_LIB NAMES mariadb mariadbclient)
  if(SYSTEM_MARIADB_C_LIB)
    set(MARIADB_C_LIB ${SYSTEM_MARIADB_C_LIB})
    message(STATUS "Using system MariaDB C library: ${MARIADB_C_LIB}")
  else()
    message(FATAL_ERROR "MariaDB C connector library not found. Install with: sudo apt-get install libmariadb-dev")
  endif()
endif()

set(MARIADB_INCLUDE_DIRS
  ${MARIADB_INSTALL_DIR}/include
  ${MARIADB_INSTALL_DIR}/include/mariadb
)

# --------------------------------------------------------------------------------------------------
# 3. CREATE IMPORTED TARGETS
# --------------------------------------------------------------------------------------------------

# libmariadb (C connector)
add_library(libmariadb STATIC IMPORTED GLOBAL)
set_target_properties(libmariadb PROPERTIES
  IMPORTED_LOCATION ${MARIADB_C_LIB}
  INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_INCLUDE_DIRS}"
)

# libmariadbcpp (C++ connector) - DEPENDS ON C CONNECTOR
add_library(mariadbcpp STATIC IMPORTED GLOBAL)
set_target_properties(mariadbcpp PROPERTIES
  IMPORTED_LOCATION ${MARIADB_CPP_LIB}
  INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_INCLUDE_DIRS}"
  INTERFACE_LINK_LIBRARIES "libmariadb"
)

# --------------------------------------------------------------------------------------------------
# 4. TARGET SETUP
# --------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PUBLIC
  ${MARIADB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  mariadbcpp
  libmariadb
)

message(STATUS "MariaDB connector setup complete")
message(STATUS "• C++ Library: ${MARIADB_CPP_LIB}")
message(STATUS "• C Library: ${MARIADB_C_LIB}")
message(STATUS "• Include directories: ${MARIADB_INCLUDE_DIRS}")
