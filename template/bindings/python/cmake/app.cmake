message(STATUS "Building CAOS as Python extension")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Python3 REQUIRED COMPONENTS Development)

if(Python3_FOUND)
    message(STATUS "Python development package found")
    message(STATUS "  Version: ${Python3_VERSION}")
    message(STATUS "  Include: ${Python3_INCLUDE_DIRS}")
    message(STATUS "  Library: ${Python3_LIBRARIES}")
    message(STATUS "  Site Packages: ${Python3_SITEARCH}")
    message(STATUS "  Module Extension: ${Python3_MODULE_EXTENSION}")

    if(Python3_VERSION VERSION_LESS "3.6")
        message(FATAL_ERROR "Python version ${Python3_VERSION} is too old. Required: 3.6+")
    endif()

    set(BUILD_COUNTER_FILE "${CMAKE_BINARY_DIR}/build_counter.txt")
    if(EXISTS "${BUILD_COUNTER_FILE}")
        file(READ "${BUILD_COUNTER_FILE}" CAOS_BUILD_COUNT)
        string(STRIP "${CAOS_BUILD_COUNT}" CAOS_BUILD_COUNT)
        math(EXPR CAOS_BUILD_COUNT "${CAOS_BUILD_COUNT} + 1")
    else()
        set(CAOS_BUILD_COUNT 1)
    endif()

    file(WRITE "${BUILD_COUNTER_FILE}" "${CAOS_BUILD_COUNT}")

    string(TOLOWER "${CAOS_DB_BACKEND}" CAOS_DB_BACKEND_LOWER)

    string(REPLACE "." ";" PYTHON_VERSION_LIST ${Python3_VERSION})
    list(GET PYTHON_VERSION_LIST 0 PYTHON_VERSION_MAJOR)
    list(GET PYTHON_VERSION_LIST 1 PYTHON_VERSION_MINOR)
    set(PYTHON_VERSION_SHORT "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")

    configure_file(
        ${CMAKE_SOURCE_DIR}/src/include/extension_config.h.in
        ${CMAKE_BINARY_DIR}/extension_config.h
        @ONLY
    )

    add_library(${PROJECT_NAME} MODULE
      src/bindings.cpp
      src/call_context.cpp
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/include
        ${Python3_INCLUDE_DIRS}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${PROJECT_NAME}"
        SUFFIX ".so"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE libcaos ${Python3_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE CAOS_BUILD_PYTHON_BINDING)

    set(PYTHON_PACKAGE_DIR "${CMAKE_BINARY_DIR}/python_package/${PROJECT_NAME}")
    file(MAKE_DIRECTORY ${PYTHON_PACKAGE_DIR})

    # Create __init__.py for the Python package
    file(WRITE ${PYTHON_PACKAGE_DIR}/__init__.py
"\"\"\"
${PROJECT_NAME} - CAOS Native Python Bindings
==============================================

Native extension for CAOS database operations.
Backend: ${CAOS_DB_BACKEND} (${CAOS_DB_BACKEND_LOWER})
Build: ${CAOS_BUILD_COUNT}
Python: ${Python3_VERSION}

Usage:
    import ${PROJECT_NAME}
    result = ${PROJECT_NAME}.hello()
\"\"\"

from .${PROJECT_NAME} import *

__version__ = '0.1.0'
__build__ = ${CAOS_BUILD_COUNT}
__backend__ = '${CAOS_DB_BACKEND_LOWER}'
__python_version__ = '${Python3_VERSION}'

def get_build_info():
    \"\"\"Return build information.\"\"\"
    return {
        'module': '${PROJECT_NAME}',
        'version': __version__,
        'build': __build__,
        'backend': __backend__,
        'python_version': __python_version__
    }")

    # Create setup.py for pip installation
    file(WRITE ${CMAKE_BINARY_DIR}/python_package/setup.py
"from setuptools import setup, Extension
import os
import sys

# Read version from package __init__.py
with open('${PROJECT_NAME}/__init__.py', 'r') as f:
    for line in f:
        if line.startswith('__version__'):
            version = line.split('=')[1].strip().strip('\"\\'')
            break

# Platform-specific module extension
module_ext = '.so' if sys.platform != 'win32' else '.pyd'

setup(
    name='caos-${PROJECT_NAME}',
    version=version,
    description='CAOS Native Python Bindings for ${PROJECT_NAME}',
    author='CAOS Development Team',
    packages=['${PROJECT_NAME}'],
    package_data={'${PROJECT_NAME}': ['${PROJECT_NAME}' + module_ext]},
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
    ],
    python_requires='>=3.6',
)")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}-python-${CAOS_DB_BACKEND_LOWER}-${CAOS_BUILD_COUNT}${Python3_MODULE_EXTENSION}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PYTHON_PACKAGE_DIR}/${PROJECT_NAME}${Python3_MODULE_EXTENSION}
        COMMENT "Copying ${PROJECT_NAME} Python extension to dist and package directory"
        VERBATIM
    )

    install(TARGETS ${PROJECT_NAME}
            LIBRARY
            DESTINATION lib/python${PYTHON_VERSION_SHORT}/site-packages/${PROJECT_NAME}
            COMPONENT python)

    install(FILES ${PYTHON_PACKAGE_DIR}/__init__.py
            DESTINATION lib/python${PYTHON_VERSION_SHORT}/site-packages/${PROJECT_NAME}
            COMPONENT python)

    install(DIRECTORY ${PYTHON_PACKAGE_DIR}
            DESTINATION lib/python${PYTHON_VERSION_SHORT}/site-packages
            COMPONENT python)

    install(CODE "message(STATUS \"\")
                message(STATUS \"Python extension '${PROJECT_NAME}' installed to: lib/python${PYTHON_VERSION_SHORT}/site-packages/${PROJECT_NAME}/\")
                message(STATUS \"Module import: 'import ${PROJECT_NAME}'\")
                message(STATUS \"Build: ${CAOS_BUILD_COUNT}, Backend: ${CAOS_DB_BACKEND}\")
                message(STATUS \"\")
                message(STATUS \"To test the installation, run:\")
                message(STATUS \"  python${PYTHON_VERSION_SHORT} -c \\\"import ${PROJECT_NAME}; print(${PROJECT_NAME}.get_build_info())\\\"\")
                message(STATUS \"\")
                message(STATUS \"For development installation:\")
                message(STATUS \"  cd ${CMAKE_BINARY_DIR}/python_package && pip${PYTHON_VERSION_SHORT} install -e .\")
                message(STATUS \"\")"
            COMPONENT python)

    if(CMAKE_BUILD_TYPE STREQUAL "release")
        add_custom_target(${PROJECT_NAME}_package_deb
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/create_package_deb.sh ${CAOS_DB_BACKEND} ${PROJECT_NAME}
            DEPENDS libcaos ${PROJECT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Building DEB package for Python extension '${PROJECT_NAME}' with ${CAOS_DB_BACKEND} backend"
        )

        add_custom_target(${PROJECT_NAME}_distribution_tarball
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/create_distribution_tarball.sh ${CAOS_DB_BACKEND} ${PROJECT_NAME}
            DEPENDS ${PROJECT_NAME}_package_deb
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Creating distribution tarball for Python extension '${PROJECT_NAME}' with ${CAOS_DB_BACKEND} backend"
        )
    else()
        # Dummy targets
        add_custom_target(${PROJECT_NAME}_package_deb
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: CMAKE_BUILD_TYPE must be 'release' to build packages. Current value: ${CMAKE_BUILD_TYPE}"
            COMMAND false
        )

        add_custom_target(${PROJECT_NAME}_distribution_tarball
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: CMAKE_BUILD_TYPE must be 'release' to build packages. Current value: ${CMAKE_BUILD_TYPE}"
            COMMAND false
            DEPENDS ${PROJECT_NAME}_package_deb
        )
    endif()

else()
    message(FATAL_ERROR "
Python development packages not found!

Required for building CAOS Python extension.

Installation commands:
• Ubuntu/Debian:    sudo apt-get install python3-dev

After installation, re-run CMake with Python support.
")
endif()
