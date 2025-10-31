# --------------------------------------------------------------------------------------------------
# PACKAGING TARGETS
# --------------------------------------------------------------------------------------------------
if(CAOS_BUILD_PHP_EXTENSION)
  add_custom_target(php_package_deb
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/extensions/php/scripts/create_package_deb.sh
    DEPENDS caos_php
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building DEB package for PHP extension"
  )
endif()
