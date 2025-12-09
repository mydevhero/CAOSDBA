if(CMAKE_BUILD_TYPE STREQUAL "debug")
  message(STATUS "Sanitizers for app debug")

  target_compile_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fno-omit-frame-pointer
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
  )
endif()
