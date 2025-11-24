# Select CAOS_ENV ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "release" CACHE STRING "CAOS Environment." )
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "debug"
    "test"
    "release"
  )
endif()
# Select CAOS_ENV ----------------------------------------------------------------------------------

option(CAOS_USE_CACHE           "CAOS using Cache"          ON)
option(CAOS_BUILD_EXAMPLES      "CAOS build examples"       ON)
option(CAOS_USE_CROWCPP         "CAOS use Crow"             ON)
option(CAOS_BUILD_EXTENSIONS    "CAOS build bindings extension"  ON)
# option(CAOS_CHECK_COVERAGE  "CAOS check coverage" OFF)


# Select CAOS_DB_BACKEND +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(EXISTS ${CMAKE_SOURCE_DIR}/cmake/db_backend.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/db_backend.cmake)
  set(CAOS_DB_BACKEND "${DB_BACKEND}" CACHE STRING "Database backend, got default from cmake/db_backend.cmake" )
else()
  set(CAOS_DB_BACKEND "MYSQL" CACHE STRING "Database backend to use for CAOS." )
endif()

set_property(CACHE CAOS_DB_BACKEND PROPERTY STRINGS
  "POSTGRESQL"
  "MYSQL"
  "MARIADB"
)
# Select CAOS_DB_BACKEND ---------------------------------------------------------------------------



# Select CAOS_USE_CACHE ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_USE_CACHE)
  set(CAOS_CACHE_BACKEND "REDIS" CACHE STRING "Cache backend to use for CAOS." )

  set_property(CACHE CAOS_CACHE_BACKEND PROPERTY STRINGS
    "REDIS"
  )
endif()
# Select CAOS_USE_CACHE ----------------------------------------------------------------------------


# EXTENSIONS +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_EXTENSIONS)
  option(CMAKE_POSITION_INDEPENDENT_CODE                             ON)
  option(CAOS_BUILD_PHP_EXTENSION    "CAOS build php extension"      ON)
  option(CAOS_BUILD_NODE_EXTENSION   "CAOS build node.js extension"  ON)
  option(CAOS_BUILD_PYTHON_EXTENSION "CAOS build python extension"   ON)
endif()
# EXTENSIONS ---------------------------------------------------------------------------------------

if(CAOS_BUILD_EXAMPLES)
  # PHP
  if(CAOS_BUILD_PHP_EXTENSION)
    option(CAOS_BUILD_PHP_EXTENSION_EXAMPLE  "CAOS build php example"        ON)
    option(CAOS_BUILD_PHP_PACKAGE_DEB        "CAOS build php package Debian" ON)
  endif()

  # NODE
  if(CAOS_BUILD_NODE_EXTENSION)
    option(CAOS_BUILD_NODE_EXTENSION_EXAMPLE  "CAOS build node example"      ON)
    # set(CAOS_BUILD_NODE_PACKAGE_DEB        "CAOS build node package Debian" ON)
  endif()

  # PYTHON
  if(CAOS_BUILD_PYTHON_EXTENSION)
    option(CAOS_BUILD_PYTHON_EXTENSION_EXAMPLE  "CAOS build python example"  ON)
    # set(CAOS_BUILD_PYTHON_PACKAGE_DEB        "CAOS build python package Debian" ON)
  endif()
endif()

# CROWCPP
if(CAOS_USE_CROWCPP)
  if(CAOS_BUILD_EXAMPLES)
    option(CAOS_BUILD_CROWCPP_EXAMPLE "CAOS build CrowCpp example"  ON)
    # set(CAOS_BUILD_CROWCPP_PACKAGE_DEB        "CAOS build crowcpp package Debian" ON)
  endif()
endif()

# Add libcaos ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
add_subdirectory(libcaos)
add_library(repoexception INTERFACE)
target_include_directories(repoexception INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libcaos/Middleware/Repository)
target_link_libraries(repoexception INTERFACE libcaos)
# Add libcaos --------------------------------------------------------------------------------------

# EXTENSIONS +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_EXTENSIONS)
  add_subdirectory(extensions)
endif()
# EXTENSIONS ---------------------------------------------------------------------------------------

# Option CAOS_BUILD_EXAMPLES +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
# Option CAOS_BUILD_EXAMPLES -----------------------------------------------------------------------
