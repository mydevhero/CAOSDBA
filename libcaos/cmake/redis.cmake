target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CACHE_REDIS)

include(ExternalProject)

set(${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR})

ExternalProject_Add(hiredis_project
  GIT_REPOSITORY https://github.com/redis/hiredis.git
  GIT_TAG v1.3.0

  CONFIGURE_COMMAND ""

  BUILD_COMMAND make ENABLE_STATIC=1 USE_SSL=1

  INSTALL_COMMAND make install PREFIX=${CMAKE_BINARY_DIR} ENABLE_STATIC=1

  BUILD_IN_SOURCE 1

  BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/lib/libhiredis.a
)

ExternalProject_Add(redis_pp_project
  GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
  GIT_TAG 1.3.15

  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DREDIS_PLUS_PLUS_BUILD_STATIC=ON
    -DREDIS_PLUS_PLUS_BUILD_SHARED=OFF
    -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}

  INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install

  DEPENDS hiredis_project

  BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/lib/libredis++.a
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)

add_library(hiredis_static STATIC IMPORTED GLOBAL)
set_target_properties(hiredis_static PROPERTIES
  IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libhiredis.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include
)
add_dependencies(hiredis_static hiredis_project)

add_library(redispp_static STATIC IMPORTED GLOBAL)
set_target_properties(redispp_static PROPERTIES
  IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libredis++.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include
)
add_dependencies(redispp_static redis_pp_project)

target_link_libraries(${PROJECT_NAME} PUBLIC redispp_static hiredis_static)
