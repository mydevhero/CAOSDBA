cmake_minimum_required(VERSION 3.5)

include(${CMAKE_BINARY_DIR}/generated_queries/Query_Config.cmake)

# PHP EXTENSION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_PHP_EXTENSION_EXAMPLE AND QUERY_EXISTS_IQuery_Test_echoString)
  add_subdirectory(php)
endif()
# PHP_EXTENSION ------------------------------------------------------------------------------------

# NODE EXTENSION +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_NODE_EXTENSION_EXAMPLE AND QUERY_EXISTS_IQuery_Test_echoString)
  add_subdirectory(node)
endif()
# NODE EXTENSION -----------------------------------------------------------------------------------

# PYTHON EXTENSION +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_PYTHON_EXTENSION_EXAMPLE AND QUERY_EXISTS_IQuery_Test_echoString)
  add_subdirectory(python)
endif()
# PYTHON EXTENSION -----------------------------------------------------------------------------------
