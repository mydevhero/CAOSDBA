find_package(Python3 REQUIRED)

set(QUERY_DEFINITIONS_FILE "${CMAKE_SOURCE_DIR}/query_definitions.txt")
set(QUERY_GENERATOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/generate_queries.py")
set(GENERATED_QUERIES_DIR "${CMAKE_BINARY_DIR}/generated_queries")

# Verifica che il file di definizioni esista
if(NOT EXISTS "${QUERY_DEFINITIONS_FILE}")
    message(FATAL_ERROR "The query definition file was not found! Expected path: ${QUERY_DEFINITIONS_FILE}")
endif()

# Verifica che lo script Python esista
if(NOT EXISTS "${QUERY_GENERATOR_SCRIPT}")
    message(FATAL_ERROR "The query generator script was not found! Expected path: ${QUERY_GENERATOR_SCRIPT}")
endif()

# Crea la directory di output
file(MAKE_DIRECTORY "${GENERATED_QUERIES_DIR}")

# Forza CMake a ri-eseguire se questi file cambiano
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${QUERY_DEFINITIONS_FILE}
    ${QUERY_GENERATOR_SCRIPT}
)

# Esegui lo script Python a configure time
message(STATUS "Generating query interfaces from ${QUERY_DEFINITIONS_FILE}")
execute_process(
    COMMAND ${Python3_EXECUTABLE}
        ${QUERY_GENERATOR_SCRIPT}
        --definitions ${QUERY_DEFINITIONS_FILE}
        --output-dir ${GENERATED_QUERIES_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE QUERY_GEN_RESULT
    OUTPUT_VARIABLE QUERY_GEN_OUTPUT
    ERROR_VARIABLE QUERY_GEN_ERROR
)

if(NOT QUERY_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Query generation failed!\nOutput: ${QUERY_GEN_OUTPUT}\nError: ${QUERY_GEN_ERROR}")
endif()

# Definisci i path dei file generati
set(GENERATED_QUERY_DEFINITION "${GENERATED_QUERIES_DIR}/Query_Definition.hpp")
set(GENERATED_QUERY_OVERRIDE "${GENERATED_QUERIES_DIR}/Query_Override.hpp")
set(GENERATED_CACHE_FORWARDING "${GENERATED_QUERIES_DIR}/Cache_Query_Forwarding.hpp")
set(GENERATED_DATABASE_FORWARDING "${GENERATED_QUERIES_DIR}/Database_Query_Forwarding.hpp")
set(GENERATED_AUTH_CONFIG "${GENERATED_QUERIES_DIR}/AuthConfig.hpp")
set(GENERATED_QUERY_CONFIG "${GENERATED_QUERIES_DIR}/Query_Config.cmake")

# Includi la configurazione generata
include(${GENERATED_QUERY_CONFIG})

# Aggiungi la directory dei file generati agli include
target_include_directories(${PROJECT_NAME} PRIVATE
    ${GENERATED_QUERIES_DIR}
)
