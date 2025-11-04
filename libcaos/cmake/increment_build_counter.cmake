set(BUILD_COUNTER_FILE "${CMAKE_BINARY_DIR}/build_counter.txt")

# Timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# Build counter
if(EXISTS "${BUILD_COUNTER_FILE}")
    file(READ "${BUILD_COUNTER_FILE}" BUILD_VERSION)
    string(STRIP "${BUILD_VERSION}" BUILD_VERSION)
    math(EXPR BUILD_VERSION "${BUILD_VERSION} + 1")
else()
    set(BUILD_VERSION 1)
endif()

file(WRITE "${BUILD_COUNTER_FILE}" "${BUILD_VERSION}")
