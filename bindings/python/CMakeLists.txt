message(STATUS "Building CAOS as Python extension")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(caos_python LANGUAGES CXX)

target_compile_definitions(libcaos PUBLIC CAOS_BUILD_PYTHON_BINDING_EXAMPLE)

# Trova Python
find_package(Python3 REQUIRED COMPONENTS Development)

if(Python3_FOUND)
    message(STATUS "Python development package found")
    message(STATUS "  Version: ${Python3_VERSION}")
    message(STATUS "  Include: ${Python3_INCLUDE_DIRS}")
    message(STATUS "  Library: ${Python3_LIBRARIES}")

    # Crea la directory per l'estensione Python
    set(PYTHON_EXT_INSTALL_DIR ${CMAKE_BINARY_DIR}/python-ext)
    file(MAKE_DIRECTORY ${PYTHON_EXT_INSTALL_DIR})

    add_library(${PROJECT_NAME} MODULE bindings.cpp)

    include(${CMAKE_SOURCE_DIR}/libcaos/cmake/debug.cmake)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "caos_native"
        SUFFIX ".so"
    )

    # Include directories
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/libcaos
    )

    # Link
    target_link_libraries(${PROJECT_NAME} PRIVATE
        libcaos
        ${Python3_LIBRARIES}
    )

    # Flag di compilazione
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wno-deprecated-declarations
        -std=c++17
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_BUILD_PYTHON_BINDING)

    # Copia l'estensione
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PYTHON_EXT_INSTALL_DIR}
        COMMENT "Copying Python extension to local test folder"
    )

    # Crea __init__.py per package
    file(WRITE ${PYTHON_EXT_INSTALL_DIR}/__init__.py
"# CAOS Native Python Bindings
from .caos_native import *

__version__ = '0.1.0'
__all__ = ['hello', 'echo_string']")

    # Installa
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/caos_native)

else()
    message(FATAL_ERROR "
Python development packages not found!

Required for building CAOS Python extension.

Installation commands:
• Ubuntu/Debian:    sudo apt-get install python3-dev

After installation, re-run: cmake -DCAOS_BUILD_PYTHON_BINDING=ON .
")
endif()
