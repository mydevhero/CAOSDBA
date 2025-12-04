target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_POSTGRESQL)

# --------------------------------------------------------------------------------------------------
# 1. DETECT OR BUILD POSTGRESQL
# --------------------------------------------------------------------------------------------------
set(PG_PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/prebuilt/")
set(PG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/postgres")

if(EXISTS ${PG_PREBUILT_DIR}/postgres/lib/libpq.a)
  message(STATUS "Using pre-built PostgreSQL libraries")
  set(PG_INSTALL_DIR ${PG_PREBUILT_DIR}/postgres)
else()
  message(STATUS "Building PostgreSQL from submodule...")

  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_postgres.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE pg_build_result
  )

  if(NOT pg_build_result EQUAL 0)
    message(FATAL_ERROR "PostgreSQL build failed")
  endif()

  set(PG_INSTALL_DIR "${CMAKE_BINARY_DIR}/postgres-install")
endif()

# --------------------------------------------------------------------------------------------------
# 2. DETECT OR BUILD LIBPQXX
# --------------------------------------------------------------------------------------------------
set(PQXX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpqxx")

if(EXISTS ${PG_PREBUILT_DIR}/libpqxx/lib/libpqxx.a)
  message(STATUS "Using pre-built libpqxx")
  set(PQXX_INSTALL_DIR ${PG_PREBUILT_DIR}/libpqxx)
else()
  message(STATUS "Building libpqxx from submodule...")

  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_libpqxx.sh ${PG_INSTALL_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE pqxx_build_result
  )

  if(NOT pqxx_build_result EQUAL 0)
    message(FATAL_ERROR "libpqxx build failed")
  endif()

  set(PQXX_INSTALL_DIR "${CMAKE_BINARY_DIR}/libpqxx-install")
endif()


# libpq
add_library(libpq STATIC IMPORTED GLOBAL)
set_target_properties(libpq PROPERTIES
  IMPORTED_LOCATION ${PG_INSTALL_DIR}/lib/libpq.a
  INTERFACE_INCLUDE_DIRECTORIES ${PG_INSTALL_DIR}/include
)

# libpgcommon
add_library(pgcommon STATIC IMPORTED GLOBAL)
set_target_properties(pgcommon PROPERTIES
  IMPORTED_LOCATION ${PG_INSTALL_DIR}/lib/libpgcommon.a
)

# libpgcommon_shlib
add_library(pgcommon_shlib STATIC IMPORTED GLOBAL)
set_target_properties(pgcommon_shlib PROPERTIES
  IMPORTED_LOCATION ${PG_INSTALL_DIR}/lib/libpgcommon_shlib.a
)

# libpgport
add_library(pgport STATIC IMPORTED GLOBAL)
set_target_properties(pgport PROPERTIES
  IMPORTED_LOCATION ${PG_INSTALL_DIR}/lib/libpgport.a
)

# libpgport_shlib
add_library(pgport_shlib STATIC IMPORTED GLOBAL)
set_target_properties(pgport_shlib PROPERTIES
  IMPORTED_LOCATION ${PG_INSTALL_DIR}/lib/libpgport_shlib.a
)

add_library(pqxx STATIC IMPORTED GLOBAL)
set_target_properties(pqxx PROPERTIES
  IMPORTED_LOCATION ${PQXX_INSTALL_DIR}/lib/libpqxx.a
  INTERFACE_INCLUDE_DIRECTORIES ${PQXX_INSTALL_DIR}/include
  INTERFACE_LINK_LIBRARIES
    libpq
    pgcommon
    pgport
    pgcommon_shlib
    pgport_shlib
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${PG_INSTALL_DIR}/include
  ${PQXX_INSTALL_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  pqxx
  libpq
  pgcommon
  pgport
  pgcommon_shlib
  pgport_shlib
)

message(STATUS "PostgreSQL+libpqxx setup complete")
