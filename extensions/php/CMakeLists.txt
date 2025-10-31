message(STATUS "Building CAOS as PHP extension")

# find_package(PHP)
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
  message(STATUS "  Include Flags: ${PHP_INCLUDE_FLAGS}")
  message(STATUS "  Libraries: ${PHP_LIBRARIES}")

  if(PHP_VERSION VERSION_LESS "8.0")
    message(FATAL_ERROR "PHP version ${PHP_VERSION} is quite old. Consider upgrading to 8.0+")
  endif()

  separate_arguments(PHP_INCLUDE_FLAGS)

  add_library(caos_php MODULE bindings.cpp)

    set_target_properties(caos_php PROPERTIES
      PREFIX ""
      OUTPUT_NAME "caos_php"
      SUFFIX ".so"
    )

  target_link_libraries(caos_php PRIVATE libcaos ${PHP_LIBRARIES})
  target_compile_options(caos_php PRIVATE ${PHP_INCLUDE_FLAGS})
  target_compile_definitions(caos_php PUBLIC CAOS_BUILD_PHP_EXTENSION)


  set(CAOS_INI_CONTENT "extension=caos_php.so\n")
  file(WRITE ${CMAKE_BINARY_DIR}/caos.ini "${CAOS_INI_CONTENT}")

  set(PHP_EXT_INSTALL_DIR ${CMAKE_BINARY_DIR}/php-ext)
  file(MAKE_DIRECTORY ${PHP_EXT_INSTALL_DIR})

  add_custom_command(TARGET caos_php POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:caos_php> ${PHP_EXT_INSTALL_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/caos.ini ${PHP_EXT_INSTALL_DIR}/20-caos.ini
      COMMENT "Copying PHP extension and ini to local test folder"
  )

  set(PHP_CONF_D_DIR "/etc/php/${PHP_VERSION}/cli/conf.d")

  install(TARGETS caos_php LIBRARY DESTINATION ${PHP_EXTENSION_DIR})
  install(FILES ${CMAKE_BINARY_DIR}/caos.ini DESTINATION ${PHP_CONF_D_DIR})

  target_compile_definitions(caos_php PUBLIC CAOS_BUILD_PHP_EXTENSION)

else()
  message(FATAL_ERROR "
✗ PHP development packages not found!

Required for building CAOS PHP extension.

Installation commands:
• Ubuntu/Debian:    sudo apt-get install php-dev libsodium-dev libargon2-dev
• CentOS/RHEL:      sudo yum install php-devel
• Fedora:           sudo dnf install php-devel
• macOS (Homebrew): brew install php
• Windows:          Download PHP SDK or use WSL

After installation, re-run: cmake -DCAOS_BUILD_PHP_EXTENSION=ON .
")
endif()
