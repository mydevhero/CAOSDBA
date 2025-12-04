option(ENABLE_TSAN "Enable Thread Sanitizer (disables ASan)" OFF) # Do not use, it fails on redis and libmariadb
option(ENABLE_ASAN "Enable Address Sanitizer (disables TSan)" ON) # ASan come default

if(CMAKE_BUILD_TYPE STREQUAL "debug" AND NOT CAOS_TEMPLATE_CODE AND NOT CAOS_BUILD_BINDINGS)
  message(STATUS "Sanitizer on build Debug")

  target_compile_options(libcaos PUBLIC -fPIC)

  if(ENABLE_TSAN)
    message(STATUS "Enabled Thread Sanitizer (TSan). ASan and LSan disabled.")
    target_compile_options(libcaos PRIVATE -fsanitize=thread -fsanitize=undefined)
    target_link_libraries(libcaos PRIVATE -fsanitize=thread)

  elseif(ENABLE_ASAN)
      message(STATUS "Enabled Address Sanitizer (ASan/LSan). TSan disabled.")
      target_compile_options(libcaos PRIVATE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
      target_link_options(libcaos PRIVATE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
  endif()
endif()
