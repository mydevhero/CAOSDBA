message(STATUS "Building CAOS as PHP extension")

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

  message(STATUS "✓ PHP development package found")
  message(STATUS "  Version: ${PHP_VERSION}")
  message(STATUS "  Extension Dir: ${PHP_EXTENSION_DIR}")

  if(PHP_VERSION VERSION_LESS "8.0")
    message(FATAL_ERROR "PHP version ${PHP_VERSION} is quite old. Consider upgrading to 8.0+")
  endif()

  separate_arguments(PHP_INCLUDE_FLAGS)

  add_library(caos MODULE bindings.cpp)

  set_target_properties(caos PROPERTIES
    PREFIX ""
    OUTPUT_NAME "caos"
    SUFFIX ".so"
  )

  target_link_libraries(caos PRIVATE libcaos ${PHP_LIBRARIES})
  target_compile_options(caos PRIVATE ${PHP_INCLUDE_FLAGS})
  target_compile_definitions(caos PUBLIC CAOS_BUILD_PHP_EXTENSION)

  string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" PHP_VERSION_SHORT "${PHP_VERSION}")
  message(STATUS "PHP Version Short: ${PHP_VERSION_SHORT}")

  set(PHP_MODS_AVAILABLE_DIR "/etc/php/${PHP_VERSION_SHORT}/mods-available")
  set(PHP_CLI_CONF_D_DIR "/etc/php/${PHP_VERSION_SHORT}/cli/conf.d")
  set(PHP_FPM_CONF_D_DIR "/etc/php/${PHP_VERSION_SHORT}/fpm/conf.d")
  set(PHP_APACHE_CONF_D_DIR "/etc/php/${PHP_VERSION_SHORT}/apache2/conf.d")


  set(CAOS_INI_CONTENT "extension=caos.so\n")
  file(WRITE ${CMAKE_BINARY_DIR}/caos.ini "${CAOS_INI_CONTENT}")

  install(TARGETS caos
          LIBRARY
          DESTINATION ${PHP_EXTENSION_DIR}
          COMPONENT php)

  install(FILES ${CMAKE_BINARY_DIR}/caos.ini
          DESTINATION ${PHP_MODS_AVAILABLE_DIR}
          RENAME caos.ini
          COMPONENT php)

  install(CODE "message(STATUS \"\")
                message(STATUS \"✓ PHP extension installed to: ${PHP_EXTENSION_DIR}/caos.so\")
                message(STATUS \"✓ PHP ini file installed to: ${PHP_MODS_AVAILABLE_DIR}/caos.ini\")
                message(STATUS \"\")
                message(STATUS \"⚠ To enable the extension, run:\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} caos\")
                message(STATUS \"\")
                message(STATUS \"  Or enable for specific SAPI:\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} -s cli caos\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} -s fpm caos\")
                message(STATUS \"  sudo phpenmod -v ${PHP_VERSION_SHORT} -s apache2 caos\")
                message(STATUS \"\")"
          COMPONENT php)

  message(STATUS "✓ PHP extension will be installed to: ${PHP_EXTENSION_DIR}/caos.so")
  message(STATUS "✓ PHP ini file will be installed to: ${PHP_MODS_AVAILABLE_DIR}/caos.ini")

else()
  message(FATAL_ERROR "
✗ PHP development packages not found!

Required for building CAOS PHP extension.

Installation commands:
• Ubuntu/Debian:    sudo apt-get install php-dev
• CentOS/RHEL:      sudo yum install php-devel
• Fedora:           sudo dnf install php-devel
• macOS (Homebrew): brew install php
• Windows:          Download PHP SDK or use WSL

After installation, re-run: cmake -DCAOS_BUILD_PHP_EXTENSION=ON .
")
endif()
