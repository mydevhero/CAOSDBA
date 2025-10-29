target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_MYSQL)

find_library(MYSQL_CONN_LIB NAMES mysqlcppconn)
if(NOT MYSQL_CONN_LIB)
  message(FATAL_ERROR "MySQL C++ connector not found. Install with: sudo apt-get install libmysqlcppconn-dev")
else()
  message(STATUS "MySQL C++ connector found.")
endif()

include_directories(/usr/include/mysql)

target_link_libraries(${PROJECT_NAME} PRIVATE ${MYSQL_CONN_LIB})
