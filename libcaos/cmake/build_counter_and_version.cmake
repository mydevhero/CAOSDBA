set(BUILD_COUNTER_FILE "${CAOS_BINARY_DIR}/build_counter.txt")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d_%H:%M:%S")

# Build counter
if(EXISTS "${BUILD_COUNTER_FILE}")
    file(READ "${BUILD_COUNTER_FILE}" CAOS_BUILD_COUNT)
    string(STRIP "${CAOS_BUILD_COUNT}" CAOS_BUILD_COUNT)
    math(EXPR CAOS_BUILD_COUNT "${CAOS_BUILD_COUNT} + 1")
else()
    set(CAOS_BUILD_COUNT 1)
endif()

file(WRITE "${BUILD_COUNTER_FILE}" "${CAOS_BUILD_COUNT}")

# Imposta le variabili che verranno sostituite nel template
set(CAOS_BUILD_COUNT ${CAOS_BUILD_COUNT})
set(BUILD_TIMESTAMP "${BUILD_TIMESTAMP}")
set(PROJECT_NAME "${PROJECT_NAME}")
set(PROJECT_VERSION "${PROJECT_VERSION}")
set(PROJECT_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(PROJECT_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(PROJECT_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# Usa il percorso assoluto passato come variabile
configure_file(
    ${CAOS_SOURCE_DIR}/build_info.h.in
    ${CAOS_BINARY_DIR}/libcaos/build_info.h
    @ONLY
)

message(STATUS "Build counter incremented to: ${CAOS_BUILD_COUNT}")

# set(BUILD_COUNTER_FILE "${CMAKE_BINARY_DIR}/build_counter.txt")
# string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# # Build counter
# if(EXISTS "${BUILD_COUNTER_FILE}")
#     file(READ "${BUILD_COUNTER_FILE}" CAOS_BUILD_COUNT)
#     string(STRIP "${CAOS_BUILD_COUNT}" CAOS_BUILD_COUNT)
#     math(EXPR CAOS_BUILD_COUNT "${CAOS_BUILD_COUNT} + 1")
# else()
#     set(CAOS_BUILD_COUNT 1)
# endif()

# file(WRITE "${BUILD_COUNTER_FILE}" "${CAOS_BUILD_COUNT}")

# # Imposta le variabili che verranno sostituite nel template
# set(CAOS_BUILD_COUNT ${CAOS_BUILD_COUNT})
# set(BUILD_TIMESTAMP "${BUILD_TIMESTAMP}")
# set(PROJECT_NAME "${PROJECT_NAME}")
# set(PROJECT_VERSION "${PROJECT_VERSION}")
# set(PROJECT_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
# set(PROJECT_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
# set(PROJECT_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# message(STATUS "Build counter incremented to: ${CAOS_BUILD_COUNT}")

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
