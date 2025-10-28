cmake_minimum_required(VERSION 3.5)

# PHP EXTENSION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_PHP_EXTENSION)
  add_subdirectory(php)
endif()
# PHP_EXTENSION ------------------------------------------------------------------------------------

# NODE EXTENSION +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_NODE_EXTENSION)
  add_subdirectory(node)
endif()
# NODE EXTENSION -----------------------------------------------------------------------------------

# PYTHON EXTENSION +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
if(CAOS_BUILD_PYTHON_EXTENSION)
  add_subdirectory(python)
endif()
# PYTHON EXTENSION -----------------------------------------------------------------------------------
