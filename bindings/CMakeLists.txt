cmake_minimum_required(VERSION 3.5)

# PHP BINDING ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_PHP_BINDING_EXAMPLE AND QUERY_EXISTS_IQuery_Example_echoString)
  add_subdirectory(php)
endif()
# PHP_BINDING --------------------------------------------------------------------------------------

# NODE BINDING +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_NODE_BINDING_EXAMPLE AND QUERY_EXISTS_IQuery_Example_echoString)
  add_subdirectory(node)
endif()
# NODE BINDING -------------------------------------------------------------------------------------

# PYTHON BINDING +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_PYTHON_BINDING_EXAMPLE AND QUERY_EXISTS_IQuery_Example_echoString)
  add_subdirectory(python)
endif()
# PYTHON BINDING -----------------------------------------------------------------------------------
