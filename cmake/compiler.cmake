set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# We want static libs
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

# set(CMAKE_C_COMPILER "clang")
# set(CMAKE_CXX_COMPILER "clang++")

option(CAOS_USE_CCACHE "Uses CCache" OFF)
if(CAOS_USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      message(STATUS "ccache enabled")
  else()
      message(STATUS "ccache not found, install with: sudo apt install ccache")
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
