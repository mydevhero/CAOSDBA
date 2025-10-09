target_compile_definitions(${PROJECT_NAME} PUBLIC CAOS_USE_CACHE_REDIS)

find_library(HIREDIS_LIB hiredis)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)

if(NOT HIREDIS_LIB OR NOT HIREDIS_INCLUDE_DIR)
  message(FATAL_ERROR "Hiredis nor found! Install with: sudo apt-get install libhiredis-dev")
endif()

FetchContent_Declare(
  redis_plus_plus
  GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
  GIT_TAG 1.3.15
)

FetchContent_MakeAvailable(redis_plus_plus)

target_link_libraries(${PROJECT_NAME} PUBLIC redis++::redis++ ${HIREDIS_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${HIREDIS_INCLUDE_DIR})
