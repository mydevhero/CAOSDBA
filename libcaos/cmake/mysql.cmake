target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_MYSQL)

# --------------------------------------------------------------------------------------------------
# 1. FIND SYSTEM MYSQL CLIENT LIBRARY
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for MySQL client library on the system...")

message(STATUS "*************** MYSQLCLIENT_LIBRARY = ${MYSQLCLIENT_LIBRARY}")
message(STATUS "*************** MYSQLCLIENT_INCLUDE_DIR = ${MYSQLCLIENT_INCLUDE_DIR}")

find_package(PkgConfig REQUIRED)

# Cerca MySQL client
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
    message(FATAL_ERROR "\n"
        "===========================================================================\n"
        "MYSQL CLIENT LIBRARY NOT FOUND\n"
        "===========================================================================\n"
        "MySQL client library is required but was not found on your system.\n\n"
        "Install with:\n"
        "    sudo apt-get install libmysqlclient-dev\n"
        "===========================================================================\n"
    )
endif()

# --------------------------------------------------------------------------------------------------
# 2. FIND SYSTEM MYSQL CONNECTOR/C++
# --------------------------------------------------------------------------------------------------
message(STATUS "Looking for MySQL Connector/C++ on the system...")

# Cerca la libreria MySQL Connector/C++
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

    # Assicurati che mysqlcppconn linki con mysqlclient
    set_target_properties(mysqlcppconn PROPERTIES
        INTERFACE_LINK_LIBRARIES "mysqlclient;${MYSQLCPPCONN_LIBRARY}"
    )

else()
    # Se non trovato, prova a installare il pacchetto corretto
    message(STATUS "MySQL Connector/C++ not found, attempting to find installed packages...")

    # Debug: mostra cosa c'è nel sistema
    execute_process(
        COMMAND find /usr -name "*mysqlcppconn*" 2>/dev/null
        OUTPUT_VARIABLE MYSQL_FILES
    )
    message(STATUS "MySQL files found: ${MYSQL_FILES}")

    execute_process(
        COMMAND dpkg -l "*mysql*" 2>/dev/null | grep ii
        OUTPUT_VARIABLE MYSQL_PACKAGES
    )
    message(STATUS "MySQL packages installed: ${MYSQL_PACKAGES}")

    message(FATAL_ERROR "\n"
        "===========================================================================\n"
        "MYSQL CONNECTOR/C++ NOT FOUND\n"
        "===========================================================================\n"
        "MySQL Connector/C++ is required but was not found.\n\n"
        "Install one of these packages:\n\n"
        "Ubuntu/Debian:\n"
        "    sudo apt-get install libmysqlcppconn-dev\n"
        "===========================================================================\n"
    )
endif()

# --------------------------------------------------------------------------------------------------
# 3. LINK ALL
# --------------------------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE mysqlcppconn mysqlclient)

message(STATUS "MySQL client and connector setup complete using system libraries")
