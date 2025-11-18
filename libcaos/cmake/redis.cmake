target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CACHE_REDIS)

# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM HIREDIS LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for hiredis library on the system...")

find_package(PkgConfig REQUIRED)

# Cerca hiredis con diversi metodi
find_package(hiredis QUIET)

if(NOT hiredis_FOUND)
  pkg_check_modules(HIREDIS_PKG QUIET hiredis)
  if(HIREDIS_PKG_FOUND)
    message(STATUS "Found hiredis via pkg-config (version: ${HIREDIS_PKG_VERSION})")
    set(hiredis_FOUND TRUE)
  endif()
endif()

if(NOT hiredis_FOUND)
  find_library(HIREDIS_LIBRARY
    NAMES hiredis
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
  )
find_path(HIREDIS_INCLUDE_DIR
  NAMES hiredis/hiredis.h
  PATHS /usr/include /usr/local/include
)

if(HIREDIS_LIBRARY AND HIREDIS_INCLUDE_DIR)
  message(STATUS "Found hiredis via find_library")
  set(hiredis_FOUND TRUE)
endif()
endif()

if(hiredis_FOUND)
  message(STATUS "Using system hiredis library")

  # Crea target per hiredis di sistema
  add_library(hiredis_static INTERFACE IMPORTED)
  if(TARGET hiredis::hiredis)
    set_target_properties(hiredis_static PROPERTIES
      INTERFACE_LINK_LIBRARIES "hiredis::hiredis"
    )
elseif(HIREDIS_PKG_FOUND)
  set_target_properties(hiredis_static PROPERTIES
    INTERFACE_LINK_LIBRARIES "${HIREDIS_PKG_LINK_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${HIREDIS_PKG_INCLUDE_DIRS}"
  )
else()
  set_target_properties(hiredis_static PROPERTIES
    INTERFACE_LINK_LIBRARIES "${HIREDIS_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${HIREDIS_INCLUDE_DIR}"
  )
endif()

else()
  message(FATAL_ERROR "\n"
    "===========================================================================\n"
    "HIREDIS LIBRARY NOT FOUND\n"
    "===========================================================================\n"
    "hiredis library is required but was not found on your system.\n\n"
    "Please install hiredis using one of the following methods:\n\n"
    "Ubuntu/Debian:\n"
    "    sudo apt-get install libhiredis-dev\n\n"
    "===========================================================================\n"
  )
endif()

# --------------------------------------------------------------------------------------------------
# 2. DETECT OR BUILD REDIS++ FROM SOURCE
# --------------------------------------------------------------------------------------------------
set(REDIS_PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/prebuilt/")
set(REDISPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/redis-plus-plus")

# Ottieni il percorso di installazione di hiredis di sistema per redis++
if(HIREDIS_PKG_FOUND)
  # Se trovato via pkg-config, usa le directory fornite
  set(HIREDIS_INSTALL_DIR_FOR_REDISPP "")
else()
  # Altrimenti ricava la directory base dalle include/library
  get_filename_component(HIREDIS_BASE_DIR "${HIREDIS_INCLUDE_DIR}" DIRECTORY)
  set(HIREDIS_INSTALL_DIR_FOR_REDISPP "${HIREDIS_BASE_DIR}")
endif()

if(EXISTS ${REDIS_PREBUILT_DIR}/redispp/lib/libredis++.a)
  message(STATUS "Using pre-built redis-plus-plus")
  set(REDISPP_INSTALL_DIR ${REDIS_PREBUILT_DIR}/redispp)
else()
  message(STATUS "Building redis-plus-plus from submodule...")

  # Verifica che il submodule esista
  if(NOT EXISTS ${REDISPP_SOURCE_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "Redis++ source directory not found: ${REDISPP_SOURCE_DIR}\n"
      "Please initialize the submodule: git submodule update --init vendor/redis-plus-plus")
  endif()

  # Esegui lo script di build passando le informazioni di hiredis di sistema
  if(HIREDIS_PKG_FOUND)
    # Se abbiamo pkg-config, passa le variabili di ambiente
    execute_process(
      COMMAND bash -c "PKG_CONFIG_PATH=${HIREDIS_PKG_PREFIX}/lib/pkgconfig ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_redispp.sh system"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE redispp_build_result
    )
else()
  # Altrimenti passa la directory di hiredis
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_redispp.sh ${HIREDIS_INSTALL_DIR_FOR_REDISPP}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE redispp_build_result
  )
endif()

if(NOT redispp_build_result EQUAL 0)
  message(FATAL_ERROR "redis-plus-plus build failed")
endif()

set(REDISPP_INSTALL_DIR ${CMAKE_BINARY_DIR}/redispp-install)
endif()

# --------------------------------------------------------------------------------------------------
# 3. TARGETS IMPORT
# --------------------------------------------------------------------------------------------------
# redis++ (build da sorgente)
add_library(redispp_static STATIC IMPORTED GLOBAL)
set_target_properties(redispp_static PROPERTIES
  IMPORTED_LOCATION ${REDISPP_INSTALL_DIR}/lib/libredis++.a
  INTERFACE_INCLUDE_DIRECTORIES ${REDISPP_INSTALL_DIR}/include
  INTERFACE_LINK_LIBRARIES "hiredis_static"  # Link con hiredis di sistema
)

# --------------------------------------------------------------------------------------------------
# 4. DISABILITA WARNING PER LE LIBRERIE REDIS
# --------------------------------------------------------------------------------------------------
add_library(redis_suppressed_flags INTERFACE)

target_compile_options(redis_suppressed_flags INTERFACE
  -Wno-pedantic
  -Wno-unused-parameter
  -Wno-missing-field-initializers
)

# Crea wrapper target che applicano i flag soppressi
add_library(hiredis_suppressed INTERFACE)
add_library(redispp_suppressed INTERFACE)

target_link_libraries(hiredis_suppressed INTERFACE
  hiredis_static
  redis_suppressed_flags
)

target_link_libraries(redispp_suppressed INTERFACE
  redispp_static
  redis_suppressed_flags
)

# --------------------------------------------------------------------------------------------------
# 5. LINK
# --------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE redispp_suppressed hiredis_suppressed)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
  ${REDISPP_INSTALL_DIR}/include
)

message(STATUS "Redis setup complete")
message(STATUS "• hiredis: system library")
message(STATUS "• redis++: built from source")
message(STATUS "Redis++ include path: ${REDISPP_INSTALL_DIR}/include")
