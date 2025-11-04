set(SPDLOG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog")

message(STATUS "Configuring spdlog via add_subdirectory")

set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${SPDLOG_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

message(STATUS "spdlog configured successfully")
