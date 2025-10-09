target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_DB_POSTGRESQL)

message(STATUS "Fetching PostgreSQL from GitHub.")

set(PQXX_BUILD_DOC OFF CACHE BOOL "Build documentation")

FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.10.1
)

FetchContent_MakeAvailable(libpqxx)

target_link_libraries(${PROJECT_NAME} PUBLIC pqxx)
