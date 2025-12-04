target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_MYSQL)

# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM MYSQL CLIENT LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for MySQL client library on the system...")

find_package(PkgConfig REQUIRED)

find_library(MYSQLCLIENT_LIBRARY
  NAMES mysqlclient mysqlclient_r
  PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/lib64
)
find_path(MYSQLCLIENT_INCLUDE_DIR
  NAMES mysql.h
  PATHS /usr/include /usr/local/include
  PATH_SUFFIXES mysql
)

if(MYSQLCLIENT_LIBRARY AND MYSQLCLIENT_INCLUDE_DIR)
  message(STATUS "Found MySQL client: ${MYSQLCLIENT_LIBRARY}")

  add_library(mysqlclient INTERFACE IMPORTED)
  set_target_properties(mysqlclient PROPERTIES
    INTERFACE_LINK_LIBRARIES "${MYSQLCLIENT_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCLIENT_INCLUDE_DIR}"
  )

else()
  message(FATAL_ERROR "MySQL client library not found. Install with: sudo apt-get install libmysqlclient-dev")
endif()

# --------------------------------------------------------------------------------------------------
# 2. FIND SYSTEM MYSQL CONNECTOR/C++
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for MySQL Connector/C++ on the system...")

find_library(MYSQLCPPCONN_LIBRARY
    NAMES
        mysqlcppconn
        mysqlcppconn8
        mysqlcppconn-static
        mysqlcppconn8-static
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib64
        /usr/lib/mysql-cpp-connector
)

find_path(MYSQLCPPCONN_INCLUDE_DIR
    NAMES
        mysqlx/xapi.h
        mysql/jdbc.h
        jdbc.h
        mysql_connection.h
    PATHS
        /usr/include
        /usr/local/include
        /usr/include/mysql-cpp-connector
    PATH_SUFFIXES
        mysql
        mysql-connector-c++
        jdbc
)

if(MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE_DIR)
  message(STATUS "Found MySQL Connector/C++: ${MYSQLCPPCONN_LIBRARY}")
  message(STATUS "MySQL Connector/C++ include: ${MYSQLCPPCONN_INCLUDE_DIR}")

  add_library(mysqlcppconn INTERFACE IMPORTED)
  set_target_properties(mysqlcppconn PROPERTIES
    INTERFACE_LINK_LIBRARIES "${MYSQLCPPCONN_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCPPCONN_INCLUDE_DIR}"
  )


  set_target_properties(mysqlcppconn PROPERTIES
    INTERFACE_LINK_LIBRARIES "mysqlclient;${MYSQLCPPCONN_LIBRARY}"
  )

else()
  message(FATAL_ERROR "MySQL C connector/C++ library not found. Install with: sudo apt-get install libmysqlcppconn-dev")
endif()

# --------------------------------------------------------------------------------------------------
# 3. LINK ALL
# --------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE mysqlcppconn mysqlclient)

message(STATUS "MySQL client and connector setup complete using system libraries")
