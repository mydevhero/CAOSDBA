message(STATUS "Building CAOS as Node.js binding example")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(caos_node LANGUAGES CXX)

target_compile_definitions(libcaos PUBLIC CAOS_BUILD_NODE_BINDING_EXAMPLE)

# Trova Node.js e npm
find_program(NODE_EXECUTABLE node)
find_program(NPM_EXECUTABLE npm)

if(NODE_EXECUTABLE AND NPM_EXECUTABLE)
  execute_process(COMMAND ${NODE_EXECUTABLE} --version
    OUTPUT_VARIABLE NODE_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${NODE_EXECUTABLE} -p "process.platform"
    OUTPUT_VARIABLE NODE_PLATFORM
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${NODE_EXECUTABLE} -p "process.arch"
    OUTPUT_VARIABLE NODE_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "Node.js development package found")
  message(STATUS "  Version: ${NODE_VERSION}")
  message(STATUS "  Platform: ${NODE_PLATFORM}")
  message(STATUS "  Arch: ${NODE_ARCH}")

  execute_process(COMMAND ${NODE_EXECUTABLE} -p "require('path').dirname(process.execPath)"
    OUTPUT_VARIABLE NODE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(NODE_HEADERS_PATHS
  "${NODE_DIR}/../include/node"
  "/usr/include/node"
  "/usr/local/include/node"
  "/opt/homebrew/include/node"  # macOS Homebrew
  "${CMAKE_BINARY_DIR}/node_modules/node-addon-api"  # node-addon-api potrebbe avere una copia
)

set(NODE_HEADERS_DIR "")
foreach(HEADER_PATH ${NODE_HEADERS_PATHS})
  if(EXISTS "${HEADER_PATH}/node_api.h")
    set(NODE_HEADERS_DIR "${HEADER_PATH}")
    break()
  endif()
endforeach()

if(NODE_HEADERS_DIR)
  message(STATUS "Node.js headers found: ${NODE_HEADERS_DIR}")
else()
  message(FATAL_ERROR "Node.js headers not found. Tried: ${NODE_HEADERS_PATHS}")
endif()

execute_process(
  COMMAND ${NODE_EXECUTABLE} -p "require('node-addon-api').include"
  OUTPUT_VARIABLE NAPI_INCLUDE_DIR_RAW
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE NAPI_CHECK_RESULT
  ERROR_QUIET
)

if(NAPI_CHECK_RESULT EQUAL 0 AND NAPI_INCLUDE_DIR_RAW)
  string(STRIP "${NAPI_INCLUDE_DIR_RAW}" NAPI_INCLUDE_DIR)
  string(REPLACE "\n" "" NAPI_INCLUDE_DIR "${NAPI_INCLUDE_DIR}")
  string(REPLACE "\"" "" NAPI_INCLUDE_DIR "${NAPI_INCLUDE_DIR}")

  message(STATUS "node-addon-api found")
  message(STATUS "  N-API Include: ${NAPI_INCLUDE_DIR}")
else()
  message(STATUS "node-addon-api not found - installing automatically...")

  execute_process(
    COMMAND ${NPM_EXECUTABLE} install node-addon-api
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE NPM_INSTALL_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
  )

if(NPM_INSTALL_RESULT EQUAL 0)
  message(STATUS "node-addon-api installed successfully")

  execute_process(
    COMMAND ${NODE_EXECUTABLE} -p "require('node-addon-api').include"
    OUTPUT_VARIABLE NAPI_INCLUDE_DIR_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE NAPI_RECHECK_RESULT
    ERROR_QUIET
  )

if(NAPI_RECHECK_RESULT EQUAL 0 AND NAPI_INCLUDE_DIR_RAW)
  # Pulisci il path
  string(STRIP "${NAPI_INCLUDE_DIR_RAW}" NAPI_INCLUDE_DIR)
  string(REPLACE "\n" "" NAPI_INCLUDE_DIR "${NAPI_INCLUDE_DIR}")
  string(REPLACE "\"" "" NAPI_INCLUDE_DIR "${NAPI_INCLUDE_DIR}")

  message(STATUS "node-addon-api verified after installation")
  message(STATUS "  N-API Include: ${NAPI_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "node-addon-api installation failed - please install manually: npm install node-addon-api")
endif()
else()
  message(FATAL_ERROR "Failed to install node-addon-api automatically. Please install manually: npm install node-addon-api")
endif()
endif()

if(NOT IS_ABSOLUTE "${NAPI_INCLUDE_DIR}")
  set(NAPI_INCLUDE_DIR "${CMAKE_BINARY_DIR}/${NAPI_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${NODE_HEADERS_DIR}")
  message(FATAL_ERROR "Node.js headers path does not exist: ${NODE_HEADERS_DIR}")
endif()

if(NOT EXISTS "${NAPI_INCLUDE_DIR}")
  message(FATAL_ERROR "N-API include path does not exist: ${NAPI_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${NODE_HEADERS_DIR}/node_api.h")
  message(FATAL_ERROR "node_api.h not found in: ${NODE_HEADERS_DIR}")
else()
  message(STATUS "node_api.h found in Node.js headers")
endif()

if(NOT EXISTS "${NAPI_INCLUDE_DIR}/napi.h")
  message(FATAL_ERROR "napi.h not found in: ${NAPI_INCLUDE_DIR}")
else()
  message(STATUS "napi.h found in node-addon-api")
endif()

set(NODE_EXT_INSTALL_DIR ${CMAKE_BINARY_DIR}/node-ext)
file(MAKE_DIRECTORY ${NODE_EXT_INSTALL_DIR})

add_library(${PROJECT_NAME} MODULE bindings.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
  PREFIX ""
  OUTPUT_NAME "caos"
  SUFFIX ".node"
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${NODE_HEADERS_DIR}      # node_api.h
  ${NAPI_INCLUDE_DIR}      # napi.h
  ${CMAKE_SOURCE_DIR}/libcaos
)

target_link_libraries(${PROJECT_NAME} PRIVATE libcaos)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  -Wno-deprecated-declarations
  -std=c++17
  -DNODE_GYP_MODULE_NAME=caos)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  CAOS_BUILD_NODE_BINDING
  NAPI_CPP_EXCEPTIONS)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NODE_EXT_INSTALL_DIR}
  COMMENT "Copying Node.js  to local test folder"
)

# package.json for binding
set(NODE_PACKAGE_JSON_CONTENT "{
  \"name\": \"caos-native\",
  \"version\": \"0.1.0\",
  \"description\": \"Native CAOS bindings for Node.js\",
  \"main\": \"index.js\",
  \"gypfile\": false
  }")
file(WRITE ${NODE_EXT_INSTALL_DIR}/package.json "${NODE_PACKAGE_JSON_CONTENT}")

# index.js wrapper
set(NODE_INDEX_JS_CONTENT "const native = require('./caos.node');

  module.exports = {
  hello: native.hello,
  echoString: native.echoString
  };")
file(WRITE ${NODE_EXT_INSTALL_DIR}/index.js "${NODE_INDEX_JS_CONTENT}")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/node_modules/caos-native)

else()
  message(FATAL_ERROR "
    Node.js development packages not found!

    Required for building CAOS Node.js binding example.

    Installation commands:
    • Ubuntu/Debian:    sudo apt-get install nodejs npm

    After installation, re-run: cmake -DCAOS_BUILD_NODE_BINDING=ON .
    ")
endif()
