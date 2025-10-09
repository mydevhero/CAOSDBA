option(ENABLE_TSAN "Enable Thread Sanitizer (disables ASan)" OFF) # Do not use, it fails on redis and libmariadb
option(ENABLE_ASAN "Enable Address Sanitizer (disables TSan)" ON) # ASan come default

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Sanitizer on build Debug")

  if(ENABLE_TSAN)
    message(STATUS "Enabled Thread Sanitizer (TSan). ASan and LSan disabled.")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread -fsanitize=undefined)
    target_link_libraries(libcaos PUBLIC -fsanitize=thread)

  elseif(ENABLE_ASAN)
      message(STATUS "Enabled Address Sanitizer (ASan/LSan). TSan disabled.")
      target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
  endif()

endif()
