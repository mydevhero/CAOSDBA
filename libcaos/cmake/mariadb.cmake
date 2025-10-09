target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_MARIADB)


find_library(MARIADB_C_LIB NAMES mariadb mariadbclient)
find_path(MARIADB_C_INCLUDE_DIR mysql.h PATH_SUFFIXES mysql)

if(NOT MARIADB_C_LIB OR NOT MARIADB_C_INCLUDE_DIR)
  message(FATAL_ERROR "MariaDB C connector not found. Install with: sudo apt-get install libmariadb-dev")
else()
  message(STATUS "MariaDB C connector found.")
endif()


message(STATUS "Fetching mariadb-connector-cpp from GitHub.")

include(ExternalProject)
ExternalProject_Add(
  mariadb-connector-cpp
  GIT_REPOSITORY https://github.com/mariadb-corporation/mariadb-connector-cpp.git
  GIT_TAG 1.1.7
  PREFIX "${CMAKE_BINARY_DIR}/_deps/mariadb-connector-cpp"
  INSTALL_DIR "${CMAKE_BINARY_DIR}/_deps/mariadb-connector-cpp-install"
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/_deps/mariadb-connector-cpp-install
    -DSKIP_TESTING=ON
    -DSKIP_EXAMPLES=ON
  BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
  INSTALL_COMMAND ${CMAKE_COMMAND} --install .
  BUILD_ALWAYS OFF
  UPDATE_COMMAND ""
)

ExternalProject_Get_Property(mariadb-connector-cpp INSTALL_DIR)
set(MARIADB_INCLUDE_DIR "${INSTALL_DIR}/include")
set(MARIADB_LIB_DIR "${INSTALL_DIR}/lib/mariadb")

include_directories(${MARIADB_INCLUDE_DIR})

add_custom_command(
  OUTPUT ${MARIADB_LIB_DIR}/libmariadbcpp.so
  COMMAND ""
  DEPENDS mariadb-connector-cpp
)

add_custom_target(mariadbcpp DEPENDS ${MARIADB_LIB_DIR}/libmariadbcpp.so)
add_dependencies(${PROJECT_NAME} mariadbcpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${INSTALL_DIR}/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${MARIADB_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${MARIADB_LIB_DIR}/libmariadbcpp.so)
