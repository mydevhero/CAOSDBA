target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CROWCPP)

set(CROW_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/crow")

message(STATUS "🔨 Configuring CrowCpp via add_subdirectory")

# Imposta le opzioni PRIMA di add_subdirectory
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CROW_ENABLE_COMPRESSION ON CACHE BOOL "" FORCE)
set(CROW_ENABLE_SSL OFF CACHE BOOL "" FORCE)

# Usa add_subdirectory
add_subdirectory(${CROW_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Crow)

message(STATUS "✅ CrowCpp configured successfully")
