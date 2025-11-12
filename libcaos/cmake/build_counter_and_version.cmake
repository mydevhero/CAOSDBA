set(BUILD_COUNTER_FILE "${CMAKE_BINARY_DIR}/build_counter.txt")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# Build counter
if(EXISTS "${BUILD_COUNTER_FILE}")
    file(READ "${BUILD_COUNTER_FILE}" CAOS_BUILD_COUNT)
    string(STRIP "${CAOS_BUILD_COUNT}" CAOS_BUILD_COUNT)
    math(EXPR CAOS_BUILD_COUNT "${CAOS_BUILD_COUNT} + 1")
else()
    set(CAOS_BUILD_COUNT 1)
endif()

file(WRITE "${BUILD_COUNTER_FILE}" "${CAOS_BUILD_COUNT}")

# Crea manualmente il file header
set(BUILD_INFO_H_CONTENT "
// File generato automaticamente - non modificare!
#pragma once

#define CAOS_BUILD_COUNT ${CAOS_BUILD_COUNT}
#define CAOS_BUILD_TIMESTAMP \"${BUILD_TIMESTAMP}\"
#define CAOS_NAME \"@PROJECT_NAME@\"
#define CAOS_VERSION \"@PROJECT_VERSION@\"
#define CAOS_VERSION_MAJOR \"@PROJECT_VERSION_MAJOR@\"
#define CAOS_VERSION_MINOR \"@PROJECT_VERSION_MINOR@\"
#define CAOS_VERSION_PATCH \"@PROJECT_VERSION_PATCH@\"
")

# Scrivi il file header manualmente
file(WRITE "${CMAKE_BINARY_DIR}/build_info.h" "${BUILD_INFO_H_CONTENT}")

message(STATUS "Build counter incremented to: ${CAOS_BUILD_COUNT}")

# Include generated directory
# target_include_directories(${PROJECT_NAME} PUBLIC
#     "${CMAKE_BINARY_DIR}/generated"
# )


# set(BUILD_COUNTER_FILE "${CMAKE_BINARY_DIR}/build_counter.txt")

# # Timestamp
# string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# # Build counter
# if(EXISTS "${BUILD_COUNTER_FILE}")
#     file(READ "${BUILD_COUNTER_FILE}" BUILD_VERSION)
#     string(STRIP "${BUILD_VERSION}" BUILD_VERSION)
#     math(EXPR BUILD_VERSION "${BUILD_VERSION} + 1")
# else()
#     set(BUILD_VERSION 1)
# endif()

# file(WRITE "${BUILD_COUNTER_FILE}" "${BUILD_VERSION}")
