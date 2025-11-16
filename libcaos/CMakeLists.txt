cmake_minimum_required(VERSION 3.5)

include (cmake/project_version.cmake)

project(libcaos VERSION ${PROJECT_VERSION} LANGUAGES CXX)

include_directories("${CMAKE_BINARY_DIR}")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/libcaos")


message(STATUS "Build counter incremented to: ${CAOS_BUILD_COUNT}-${CAOS_ROOT_DIR}")
# set(CAOS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# include (cmake/build_counter_and_version.cmake)
add_custom_target(increment_build ALL
  COMMAND ${CMAKE_COMMAND}
    -DCAOS_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DCAOS_BINARY_DIR=${CMAKE_BINARY_DIR}
    -DPROJECT_NAME=${PROJECT_NAME}
    -DPROJECT_VERSION=${PROJECT_VERSION}
    -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_counter_and_version.cmake
  BYPRODUCTS "${CMAKE_BINARY_DIR}/build_counter.txt"
  COMMENT "Incrementing build counter"
)



if(CAOS_DB_BACKEND STREQUAL "POSTGRESQL")
  list(APPEND POSTGRESQL_SOURCES
    Middleware/Repository/Database/PostgreSQL/PostgreSQL.cpp
    Middleware/Repository/Database/PostgreSQL/PostgreSQL.hpp
    Middleware/Repository/Database/PostgreSQL/Query.hpp
  )
endif()

if(CAOS_DB_BACKEND STREQUAL "MYSQL")
  list(APPEND MYSQL_SOURCES
    Middleware/Repository/Database/MySQL/MySQL.hpp
    Middleware/Repository/Database/MySQL/MySQL.cpp
    Middleware/Repository/Database/MySQL/Query.hpp
  )
endif()

if(CAOS_DB_BACKEND STREQUAL "MARIADB")
  list(APPEND MARIADB_SOURCES
    Middleware/Repository/Database/MariaDB/MariaDB.cpp
    Middleware/Repository/Database/MariaDB/MariaDB.hpp
    Middleware/Repository/Database/MariaDB/Query.hpp
  )
endif()

if(CAOS_USE_CROWCPP)
  list(APPEND CROWCPP_SOURCES
    CrowCpp.cpp
    include/CrowCpp.hpp
    Middleware/Repository.hpp
  )
endif()

if(CAOS_USE_CACHE)
  list(APPEND CACHE_SOURCES
    Middleware/Repository/Cache/Cache.hpp
    Middleware/Repository/Cache/Cache.cpp
    Middleware/Repository/Cache/Query.hpp
  )

  if(CAOS_CACHE_BACKEND STREQUAL "REDIS")
    list(APPEND REDIS_SOURCES
      Middleware/Repository/Cache/Redis/Redis.hpp
      Middleware/Repository/Cache/Redis/Redis.cpp
      Middleware/Repository/Cache/Redis/Query.hpp
    )
  endif()
endif()

add_library(${PROJECT_NAME} STATIC ${PROJECT_NAME}.cpp
  config_validation.hpp
  logo.hpp
  common.hpp

  terminal_options.cpp
  include/terminal_options.hpp
  include/Filter.hpp
  include/Filter/Auth.hpp
  include/Filter/Auth/Token.hpp

  Middleware/Repository/Exception.hpp
  Middleware/Repository/IRepository.hpp
  Middleware/Repository.hpp
  Middleware/Repository/IQuery.hpp
  Middleware/Repository/Database/Database.cpp
  Middleware/Repository/Database/Database.hpp
  Middleware/Repository/Database/Query.hpp
  ${POSTGRESQL_SOURCES}
  ${MYSQL_SOURCES}
  ${MARIADB_SOURCES}
  ${CROWCPP_SOURCES}
  ${CACHE_SOURCES}
  ${REDIS_SOURCES}
)
include(cmake/compiler.cmake)
include(cmake/query_code_generation.cmake)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if(CMAKE_BUILD_TYPE STREQUAL "debug")
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_ENV_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "test")
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_ENV_TEST)
elseif(CMAKE_BUILD_TYPE STREQUAL "release")
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_ENV_RELEASE)
else()
  message(FATAL_ERROR "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} must be equal to: debug, test or release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "test")
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_ENV_ALT)                                   # Alternative Environment for debug and test
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_ENV="${CMAKE_BUILD_TYPE}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/config.hpp)

# Option CAOS_CHECK_COVERAGE +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# if(CAOS_CHECK_COVERAGE)
#   include(cmake/coverage.cmake)
# endif()
# Option CAOS_CHECK_COVERAGE -----------------------------------------------------------------------





# Option CAOS_BUILD_TEST +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_ENV_TEST OR CMAKE_BUILD_TYPE STREQUAL "test")
  add_subdirectory(test)
endif()
# Option CAOS_BUILD_TEST ---------------------------------------------------------------------------



add_custom_target(generate_version DEPENDS ${VERSION_HEADER_OUT})
add_dependencies(${PROJECT_NAME} generate_version)


include(cmake/debug.cmake)

# cxxopts
include(cmake/cxxopts.cmake)

# fmt
include (cmake/fmt.cmake)

# spdlog
include (cmake/spdlog.cmake)

# CrowCpp
if(CAOS_USE_CROWCPP)
  include(cmake/crowcpp.cmake)
endif()

# --------------------------------------------------------------------------------------------------
# Cache
# --------------------------------------------------------------------------------------------------
if(CAOS_USE_CACHE)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CACHE)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_CACHE_BACKEND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Repository/Cache)

  # Redis
  if(CAOS_CACHE_BACKEND STREQUAL "REDIS")
    include("cmake/redis.cmake")
  endif()

endif()
# --------------------------------------------------------------------------------------------------

if(CAOS_BUILD_PHP_EXTENSION)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_BUILD_PHP_EXTENSION)
endif()


# --------------------------------------------------------------------------------------------------
# Database
# --------------------------------------------------------------------------------------------------

# PostgreSQL
if(CAOS_DB_BACKEND STREQUAL "POSTGRESQL")
  include("cmake/postgresql.cmake")
endif()

# MySQL
if(CAOS_DB_BACKEND STREQUAL "MYSQL")
  include("cmake/mysql.cmake")
endif()

# MariaDB
if(CAOS_DB_BACKEND STREQUAL "MARIADB")
  include("cmake/mariadb.cmake")
endif()
# --------------------------------------------------------------------------------------------------


target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/repository
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Repository
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Repository/Database
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/libcaos
)

target_link_libraries(${PROJECT_NAME} PUBLIC pthread)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

