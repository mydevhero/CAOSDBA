target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CACHE_REDIS)

# --------------------------------------------------------------------------------------------------
# 1. DETECT OR BUILD HIREDIS
# --------------------------------------------------------------------------------------------------
set(REDIS_PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/prebuilt/")
set(HIREDIS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/hiredis")

if(EXISTS ${REDIS_PREBUILT_DIR}/hiredis/lib/libhiredis.a)
  message(STATUS "Using pre-built hiredis")
  set(HIREDIS_INSTALL_DIR ${REDIS_PREBUILT_DIR}/hiredis)
else()
  message(STATUS "Building hiredis from submodule...")
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_hiredis.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE hiredis_build_result
  )
  if(NOT hiredis_build_result EQUAL 0)
    message(FATAL_ERROR "hiredis build failed")
  endif()
  set(HIREDIS_INSTALL_DIR ${CMAKE_BINARY_DIR}/hiredis-install)
endif()

# --------------------------------------------------------------------------------------------------
# 2. DETECT OR BUILD REDIS++
# --------------------------------------------------------------------------------------------------
set(REDISPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/redis-plus-plus")

if(EXISTS ${REDIS_PREBUILT_DIR}/redispp/lib/libredis++.a)
  message(STATUS "Using pre-built redis-plus-plus")
  set(REDISPP_INSTALL_DIR ${REDIS_PREBUILT_DIR}/redispp)
else()
  message(STATUS "Building redis-plus-plus from submodule...")
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_redispp.sh ${HIREDIS_INSTALL_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE redispp_build_result
  )
  if(NOT redispp_build_result EQUAL 0)
    message(FATAL_ERROR "redis-plus-plus build failed")
  endif()
  set(REDISPP_INSTALL_DIR ${CMAKE_BINARY_DIR}/redispp-install)
endif()

# --------------------------------------------------------------------------------------------------
# 3. TARGETS IMPORT
# --------------------------------------------------------------------------------------------------
# hiredis
add_library(hiredis_static STATIC IMPORTED GLOBAL)
set_target_properties(hiredis_static PROPERTIES
  IMPORTED_LOCATION ${HIREDIS_INSTALL_DIR}/lib/libhiredis.a
  INTERFACE_INCLUDE_DIRECTORIES ${HIREDIS_INSTALL_DIR}/include
)

# redis++
add_library(redispp_static STATIC IMPORTED GLOBAL)
set_target_properties(redispp_static PROPERTIES
  IMPORTED_LOCATION ${REDISPP_INSTALL_DIR}/lib/libredis++.a
  INTERFACE_INCLUDE_DIRECTORIES ${REDISPP_INSTALL_DIR}/include
  INTERFACE_LINK_LIBRARIES "hiredis_static"
)

# --------------------------------------------------------------------------------------------------
# 4. DISABILITA WARNING PER LE LIBRERIE REDIS
# --------------------------------------------------------------------------------------------------
add_library(redis_suppressed_flags INTERFACE)

target_compile_options(redis_suppressed_flags INTERFACE
  -Wno-pedantic
  -Wno-unused-parameter
  -Wno-missing-field-initializers
)

# Crea wrapper target che applicano i flag soppressi
add_library(hiredis_suppressed INTERFACE)
add_library(redispp_suppressed INTERFACE)

target_link_libraries(hiredis_suppressed INTERFACE
  hiredis_static
  redis_suppressed_flags
)

target_link_libraries(redispp_suppressed INTERFACE
  redispp_static
  redis_suppressed_flags
)

# --------------------------------------------------------------------------------------------------
# 5. LINK
# --------------------------------------------------------------------------------------------------
# target_link_libraries(${PROJECT_NAME} PRIVATE redispp_static hiredis_static)

# target_include_directories(${PROJECT_NAME} PUBLIC
#   ${HIREDIS_INSTALL_DIR}/include
#   ${REDISPP_INSTALL_DIR}/include
# )

target_link_libraries(${PROJECT_NAME} PRIVATE redispp_suppressed hiredis_suppressed)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
  ${HIREDIS_INSTALL_DIR}/include
  ${REDISPP_INSTALL_DIR}/include
)

message(STATUS "Redis setup complete")
message(STATUS "Redis include paths:")
message(STATUS "  hiredis: ${HIREDIS_INSTALL_DIR}/include")
message(STATUS "  redis++: ${REDISPP_INSTALL_DIR}/include")

















# target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CACHE_REDIS)

# # --------------------------------------------------------------------------------------------------
# # 1. DETECT OR BUILD HIREDIS
# # --------------------------------------------------------------------------------------------------
# set(REDIS_PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/prebuilt/")
# set(HIREDIS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/hiredis")

# if(EXISTS ${REDIS_PREBUILT_DIR}/hiredis/lib/libhiredis.a)
#   message(STATUS "Using pre-built hiredis")
#   set(HIREDIS_INSTALL_DIR ${REDIS_PREBUILT_DIR}/hiredis)
# else()
#   message(STATUS "Building hiredis from submodule...")
#   execute_process(
#     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_hiredis.sh
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     RESULT_VARIABLE hiredis_build_result
#   )
#   if(NOT hiredis_build_result EQUAL 0)
#     message(FATAL_ERROR "hiredis build failed")
#   endif()
#   set(HIREDIS_INSTALL_DIR ${CMAKE_BINARY_DIR}/hiredis-install)
# endif()

# # --------------------------------------------------------------------------------------------------
# # 2. DETECT OR BUILD REDIS++
# # --------------------------------------------------------------------------------------------------
# set(REDISPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/redis-plus-plus")

# if(EXISTS ${REDIS_PREBUILT_DIR}/redispp/lib/libredis++.a)
#   message(STATUS "Using pre-built redis-plus-plus")
#   set(REDISPP_INSTALL_DIR ${REDIS_PREBUILT_DIR}/redispp)
# else()
#   message(STATUS "Building redis-plus-plus from submodule...")
#   execute_process(
#     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vendor/build-scripts/build_redispp.sh ${HIREDIS_INSTALL_DIR}
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     RESULT_VARIABLE redispp_build_result
#   )
#   if(NOT redispp_build_result EQUAL 0)
#     message(FATAL_ERROR "redis-plus-plus build failed")
#   endif()
#   set(REDISPP_INSTALL_DIR ${CMAKE_BINARY_DIR}/redispp-install)
# endif()

# # --------------------------------------------------------------------------------------------------
# # 3. TARGETS IMPORT
# # --------------------------------------------------------------------------------------------------
# # hiredis
# add_library(hiredis_static STATIC IMPORTED GLOBAL)
# set_target_properties(hiredis_static PROPERTIES
#   IMPORTED_LOCATION ${HIREDIS_INSTALL_DIR}/lib/libhiredis.a
#   INTERFACE_INCLUDE_DIRECTORIES ${HIREDIS_INSTALL_DIR}/include
# )

# # redis++
# add_library(redispp_static STATIC IMPORTED GLOBAL)
# set_target_properties(redispp_static PROPERTIES
#   IMPORTED_LOCATION ${REDISPP_INSTALL_DIR}/lib/libredis++.a
#   INTERFACE_INCLUDE_DIRECTORIES ${REDISPP_INSTALL_DIR}/include
#   INTERFACE_LINK_LIBRARIES "hiredis_static"
# )

# # --------------------------------------------------------------------------------------------------
# # 4. LINK
# # --------------------------------------------------------------------------------------------------
# target_link_libraries(${PROJECT_NAME} PRIVATE redispp_static hiredis_static)

# target_include_directories(${PROJECT_NAME} PUBLIC
#   ${HIREDIS_INSTALL_DIR}/include
#   ${REDISPP_INSTALL_DIR}/include
# )

# message(STATUS "Redis setup complete")
# message(STATUS "Redis include paths:")
# message(STATUS "  hiredis: ${HIREDIS_INSTALL_DIR}/include")
# message(STATUS "  redis++: ${REDISPP_INSTALL_DIR}/include")
