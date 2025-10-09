set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(${CMAKE_C_COMPILER} MATCHES "gcc$" AND ${CMAKE_CXX_COMPILER} MATCHES "g\\+\\+$")
  message(STATUS "Using GNU Compiler Suite (GCC/G++).")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Compilang for Debug.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Og -rdynamic -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Og -rdynamic -fno-omit-frame-pointer")
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic -Werror -Wunused)

  else()
    message(STATUS "Compilang for Release.")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fPIC -flto -march=native -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE   "-O3 -DNDEBUG -fPIC -flto -march=native -DNDEBUG")

    if(CMAKE_HOST_PLATFORM MATCHES "Linux")
      execute_process(COMMAND nproc OUTPUT_VARIABLE LTO_JOBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
      set(LTO_JOBS 4) # Fallback
    endif()
    target_link_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:-flto=${LTO_JOBS}>)
  endif()

elseif(${CMAKE_C_COMPILER} MATCHES "clang$" AND ${CMAKE_CXX_COMPILER} MATCHES "clang\\+\\+$")
  message(STATUS "Using clang Compiler Suite.")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Compilang for Debug.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Og -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Og -fno-omit-frame-pointer")
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic -Werror -Wunused)

  else()
    message(STATUS "Compilang for Release.")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fPIC -flto -march=native -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE   "-O3 -DNDEBUG -fPIC -flto -march=native -DNDEBUG")
    if(CMAKE_HOST_PLATFORM MATCHES "Linux")
      execute_process(COMMAND nproc OUTPUT_VARIABLE LTO_JOBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
      set(LTO_JOBS 4) # Fallback
    endif()
    target_link_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:-flto=${LTO_JOBS}>)
  endif()
endif()
