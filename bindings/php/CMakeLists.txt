message(STATUS "Building CAOS as PHP binding example")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME} LANGUAGES CXX)

target_compile_definitions(libcaos PUBLIC CAOS_BUILD_PHP_BINDING_EXAMPLE)

find_program(PHP_CONFIG php-config)

if(PHP_CONFIG)
  execute_process(COMMAND ${PHP_CONFIG} --includes
    OUTPUT_VARIABLE PHP_INCLUDE_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${PHP_CONFIG} --libs
    OUTPUT_VARIABLE PHP_LIBRARIES
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${PHP_CONFIG} --version
    OUTPUT_VARIABLE PHP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${PHP_CONFIG} --extension-dir
    OUTPUT_VARIABLE PHP_EXTENSION_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "PHP development package found")
  message(STATUS "  Version: ${PHP_VERSION}")
  message(STATUS "  Extension Dir: ${PHP_EXTENSION_DIR}")

  if(PHP_VERSION VERSION_LESS "8.0")
    message(FATAL_ERROR "PHP version ${PHP_VERSION} is too old. Required: 8.0+")
  endif()

  separate_arguments(PHP_INCLUDE_FLAGS)

  add_library(${PROJECT_NAME} MODULE bindings.cpp)

  set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "caos"
    SUFFIX ".so"
  )

  target_link_libraries(${PROJECT_NAME} PRIVATE libcaos ${PHP_LIBRARIES})
  target_compile_options(${PROJECT_NAME} PRIVATE ${PHP_INCLUDE_FLAGS})
  target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_BUILD_PHP_BINDING)

  string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" PHP_VERSION_SHORT "${PHP_VERSION}")
  message(STATUS "PHP Version Short: ${PHP_VERSION_SHORT}")

  set(PHP_MODS_AVAILABLE_DIR "/etc/php/${PHP_VERSION_SHORT}/mods-available")
  set(PHP_CLI_CONF_D_DIR "/etc/php/${PHP_VERSION_SHORT}/cli/conf.d")
  set(PHP_FPM_CONF_D_DIR "/etc/php/${PHP_VERSION_SHORT}/fpm/conf.d")
  set(PHP_APACHE_CONF_D_DIR "/etc/php/${PHP_VERSION_SHORT}/apache2/conf.d")

  set(CAOS_INI_CONTENT "extension=caos.so\n")
  file(WRITE ${CMAKE_BINARY_DIR}/caos.ini "${CAOS_INI_CONTENT}")

  install(TARGETS ${PROJECT_NAME}
          LIBRARY
          DESTINATION ${PHP_EXTENSION_DIR}
          COMPONENT php)

  install(FILES ${CMAKE_BINARY_DIR}/caos.ini
          DESTINATION ${PHP_MODS_AVAILABLE_DIR}
          RENAME caos.ini
          COMPONENT php)

  install(CODE "message(STATUS \"\")
                message(STATUS \"PHP binding example installed to: ${PHP_EXTENSION_DIR}/caos.so\")
                message(STATUS \"PHP ini file installed to: ${PHP_MODS_AVAILABLE_DIR}/caos.ini\")
                message(STATUS \"\")
                message(STATUS \"To enable the extension, run:\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} caos\")
                message(STATUS \"\")
                message(STATUS \"Or enable for specific SAPI:\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} -s cli caos\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} -s fpm caos\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} -s apache2 caos\")
                message(STATUS \"\")"
          COMPONENT php)

  message(STATUS "PHP binding example will be installed to: ${PHP_EXTENSION_DIR}/caos.so")
  message(STATUS "PHP ini file will be installed to: ${PHP_MODS_AVAILABLE_DIR}/caos.ini")

  if(CAOS_BUILD_PHP_PACKAGE_DEB)
    add_custom_target(php_package_deb
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/create_package_deb.sh ${CAOS_DB_BACKEND}
      DEPENDS libcaos ${PROJECT_NAME}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Building DEB package for PHP binding example with ${CAOS_DB_BACKEND} backend"
    )

    add_custom_target(php_distribution_tarball
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/create_distribution_tarball.sh ${CAOS_DB_BACKEND}
      DEPENDS php_package_deb
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating distribution tarball with ${CAOS_DB_BACKEND} backend"
    )
  endif()

else()
  message(FATAL_ERROR "
PHP development packages not found!

Required for building CAOS PHP binding example.

Installation commands:
Ubuntu/Debian: sudo apt-get install php-dev

After installation, re-run: cmake -DCAOS_BUILD_PHP_BINDING=ON .
")
endif()
