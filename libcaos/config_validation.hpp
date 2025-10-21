#pragma once

static_assert(__cplusplus >= 201703L, "Requires C++17");

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define APPEND_ERRMSG_NON_EMPTY     " must be defined and non-empty"
#define APPEND_ERRMSG_OUT_OF_RANGE  " out of range"
#define APPEND_ERRMSG_AT_LEAST      " must be at least equal to "
#define APPEND_ERRMSG_NON_NULL      " must be non-null"
#define APPEND_ERRMSG_NON_BOOL      " must be boolean"


constexpr bool is_non_null_string(const char* str = nullptr) { return str != nullptr; }
constexpr bool is_non_empty_string(const char* str = nullptr) { return str[0] != '\0'; }
constexpr bool is_non_null_and_non_empty_string(const char* str = nullptr) { return is_non_null_string(str) && is_non_empty_string(str); }
// ?
// constexpr bool string_has_a_value(const char* str = nullptr) { return str[0] == '\0' || strlen(str) > 0; }
// ?
// constexpr bool is_non_null_and_has_a_value(const char* str = nullptr) { return is_non_null_string(str) && string_has_a_value(str); }
constexpr bool is_in_range(int min, int max, int num) { return num >= min && num <= max; }
constexpr bool is_valid_env(const char* str = nullptr)
{
  return str != nullptr &&
         (std::string_view(str) == "debug" ||
          std::string_view(str) == "test" ||
          std::string_view(str) == "release");
}
constexpr bool is_valid_severity(const char* str = nullptr)
{
  return str != nullptr &&
         (std::string_view(str) == "trace"    ||
          std::string_view(str) == "debug"    ||
          std::string_view(str) == "info"     ||
          std::string_view(str) == "warn"     ||
          std::string_view(str) == "err"      ||
          std::string_view(str) == "critical" ||
          std::string_view(str) == "off");
}

template<int Value = -1>
constexpr bool is_bool()
{
  return Value != -1 && (Value==0 || Value==1);
}

template<int Value = -999999>
constexpr bool is_non_null_number()
{
  return Value != -999999;
}

// ?
// template<int Value = -999999>
// constexpr bool is_gt_than_number(int min) /* to delete */
// {
//   return Value > min;
// }

template<int Value = -999999>
constexpr bool numberAtLeast(int min)
{
  return min > Value;
}

// ?
// template<int Value = -999999>
// constexpr bool is_ls_than_number(int max)
// {
//   return Value < max;
// }

// ?
// template<int Value = -999999>
// constexpr bool is_non_null_and_gt_than_number(int min)
// {
//   return is_non_null_number<Value>() && is_gt_than_number<Value>(min);
// }

template<int Value = -999999>
constexpr bool is_number_non_null_and_at_least(int min)
{
  return is_non_null_number<Value>() && !numberAtLeast<Value>(min);
}


// ?
// template<int Value = -999999>
// constexpr bool is_non_null_and_in_range_number(int min, int max)
// {
//   return is_non_null_number<Value>() && is_gt_than_number<Value>(min) && is_ls_than_number<Value>(max);
// }



// Default values ----------------------------------------------------------------------------------

// Threads -----------------------------------------------------------------------------------------
#define CAOS_DEFAULT_THREADS_VALUE_DEFAULT    1
#define CAOS_DEFAULT_THREADS_VALUE_LIMIT_MIN  1

#ifndef CAOS_DEFAULT_THREADS_VALUE
  #define CAOS_DEFAULT_THREADS_VALUE CAOS_DEFAULT_THREADS_VALUE_DEFAULT
#endif

#define CAOS_DEFAULT_THREADS_VALUE_ERRMSG "CAOS_DEFAULT_THREADS_VALUE" APPEND_ERRMSG_AT_LEAST
static_assert(is_number_non_null_and_at_least<CAOS_DEFAULT_THREADS_VALUE>(CAOS_DEFAULT_THREADS_VALUE_LIMIT_MIN), CAOS_DEFAULT_THREADS_VALUE_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Unprivileged port min ---------------------------------------------------------------------------
#define DEFAULT_UNPRIVILEGED_PORT_MIN_DEFAULT   1024
#define DEFAULT_UNPRIVILEGED_PORT_MIN_LIMIT_MIN 1024

#ifndef DEFAULT_UNPRIVILEGED_PORT_MIN
  #define DEFAULT_UNPRIVILEGED_PORT_MIN DEFAULT_UNPRIVILEGED_PORT_MIN_DEFAULT
#endif

#define DEFAULT_UNPRIVILEGED_PORT_MIN_ERRMSG "DEFAULT_UNPRIVILEGED_PORT_MIN" APPEND_ERRMSG_AT_LEAST
static_assert(is_number_non_null_and_at_least<DEFAULT_UNPRIVILEGED_PORT_MIN>(DEFAULT_UNPRIVILEGED_PORT_MIN_LIMIT_MIN), DEFAULT_UNPRIVILEGED_PORT_MIN_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Unprivileged port max ---------------------------------------------------------------------------
#define DEFAULT_UNPRIVILEGED_PORT_MAX_DEFAULT 49151
#define DEFAULT_UNPRIVILEGED_PORT_MAX_LIMIT_MIN 1024

#ifndef DEFAULT_UNPRIVILEGED_PORT_MAX
  #define DEFAULT_UNPRIVILEGED_PORT_MAX DEFAULT_UNPRIVILEGED_PORT_MAX_DEFAULT
#endif

#define DEFAULT_UNPRIVILEGED_PORT_MAX_ERRMSG "DEFAULT_UNPRIVILEGED_PORT_MAX" APPEND_ERRMSG_AT_LEAST
static_assert(is_number_non_null_and_at_least<DEFAULT_UNPRIVILEGED_PORT_MAX>(DEFAULT_UNPRIVILEGED_PORT_MAX_LIMIT_MIN), DEFAULT_UNPRIVILEGED_PORT_MAX_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Check unprivileged port min/max order
#if DEFAULT_UNPRIVILEGED_PORT_MIN >= DEFAULT_UNPRIVILEGED_PORT_MAX
  #error "DEFAULT_UNPRIVILEGED_PORT_MIN must be < than DEFAULT_UNPRIVILEGED_PORT_MAX"
#endif
//--------------------------------------------------------------------------------------------------



// Environment -------------------------------------------------------------------------------------
#ifdef CMAKE_BUILD_TYPE
  static_assert(is_non_null_and_non_empty_string(CAOS_ENV), "CMAKE_BUILD_TYPE must be defined and non-empty");
#endif

static_assert(is_valid_env(CAOS_ENV), "CMAKE_BUILD_TYPE must be equal to \"debug\", \"test\" or \"release\" (case sensitive!). Add -DCMAKE_BUILD_TYPE:STRING=${ENV} on cmake");
//--------------------------------------------------------------------------------------------------

// LOG Severity ------------------------------------------------------------------------------------
#ifdef CAOS_ENV_LOG_SEVERITY_NAME
  static_assert(is_non_null_and_non_empty_string(CAOS_ENV_LOG_SEVERITY_NAME), "CAOS_ENV_LOG_SEVERITY_NAME must be defined and non-empty");
#else
  #define CAOS_ENV_LOG_SEVERITY_NAME "CAOS_SEVERITY"
#endif

#ifdef CAOS_OPT_LOG_SEVERITY_NAME
  static_assert(is_non_null_and_non_empty_string(CAOS_OPT_LOG_SEVERITY_NAME), "CAOS_OPT_LOG_SEVERITY_NAME must be defined and non-empty");
#else
  #define CAOS_OPT_LOG_SEVERITY_NAME "severity"
#endif


// TODO: static_assert(is_non_null_and_has_a_value(CAOS_SEVERITY_LEVEL_BEFORE_LOG_START), "CAOS_SEVERITY_LEVEL_BEFORE_LOG_START must be defined and must have a value (even empty)");


#ifdef CAOS_ENV_DEBUG
  #ifndef CAOS_LOG_SEVERITY
    #define CAOS_LOG_SEVERITY "trace"
  #endif
#elif defined(CAOS_ENV_TEST)
  #ifndef CAOS_LOG_SEVERITY
    #define CAOS_LOG_SEVERITY "debug"
  #endif
#elif defined(CAOS_ENV_RELEASE)
  #ifndef CAOS_LOG_SEVERITY
    #define CAOS_LOG_SEVERITY "warn"
  #endif
#endif
static_assert(is_valid_severity(CAOS_LOG_SEVERITY), "CAOS_LOG_SEVERITY, allowed values: trace, debug, info, warn, err, critical, off");


// Log queue ---------------------------------------------------------------------------------------
#define CAOS_LOG_QUEUE_DEFAULT    8192
#define CAOS_LOG_QUEUE_LIMIT_MIN  1024

#ifndef CAOS_LOG_QUEUE
  #define CAOS_LOG_QUEUE CAOS_LOG_QUEUE_DEFAULT
#endif

#define CAOS_LOG_QUEUE_ERRMSG "CAOS_LOG_QUEUE" APPEND_ERRMSG_AT_LEAST
static_assert(is_number_non_null_and_at_least<CAOS_LOG_QUEUE>(CAOS_LOG_QUEUE_LIMIT_MIN), CAOS_LOG_QUEUE_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Log thread count --------------------------------------------------------------------------------
#define CAOS_LOG_THREAD_COUNT_DEFAULT   1
#define CAOS_LOG_THREAD_COUNT_LIMIT_MIN 1

#ifndef CAOS_LOG_THREAD_COUNT
  #define CAOS_LOG_THREAD_COUNT CAOS_LOG_THREAD_COUNT_DEFAULT
#endif

#define CAOS_LOG_THREAD_COUNT_ERRMSG "CAOS_LOG_THREAD_COUNT" APPEND_ERRMSG_AT_LEAST
static_assert(is_number_non_null_and_at_least<CAOS_LOG_THREAD_COUNT>(CAOS_LOG_THREAD_COUNT_LIMIT_MIN), CAOS_LOG_THREAD_COUNT_ERRMSG);
//--------------------------------------------------------------------------------------------------




#ifdef CAOS_LOG_FILE
  static_assert(is_non_null_and_non_empty_string(CAOS_LOG_FILE), "CAOS_LOG_FILE must be defined and non-empty");
#else
  #define CAOS_LOG_FILE "/var/log/caos.log"
  // TODO; Create & Test file?
#endif

// CAOS_LOG_FILE_DIMENSION -------------------------------------------------------------------------
#define CAOS_LOG_FILE_DIMENSION_LIMIT_MIN 5242880                                                   /* 5M = 1024*1024*5 */
#define CAOS_LOG_FILE_DIMENSION_DEFAULT   CAOS_LOG_FILE_DIMENSION_LIMIT_MIN

#ifndef CAOS_LOG_FILE_DIMENSION
  #define CAOS_LOG_FILE_DIMENSION CAOS_LOG_FILE_DIMENSION_DEFAULT
#endif

#define CAOS_LOG_FILE_DIMENSION_ERRMSG "CAOS_LOG_FILE_DIMENSION  must be defined and be at least " TOSTRING(CAOS_LOG_FILE_DIMENSION_LIMIT_MIN) " bytes"
static_assert(is_number_non_null_and_at_least<CAOS_LOG_FILE_DIMENSION>(CAOS_LOG_FILE_DIMENSION_LIMIT_MIN), CAOS_LOG_FILE_DIMENSION_ERRMSG);
// -------------------------------------------------------------------------------------------------



// CAOS_LOG_NUMBER ---------------------------------------------------------------------------------
#define CAOS_LOG_NUMBER_LIMIT_MIN 1
#define CAOS_LOG_NUMBER_DEFAULT   5

#ifndef CAOS_LOG_NUMBER
  #define CAOS_LOG_NUMBER CAOS_LOG_NUMBER_DEFAULT
#endif

#define CAOS_LOG_NUMBER_ERRMSG "CAOS_LOG_NUMBER  must be at least " TOSTRING(CAOS_LOG_NUMBER_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_LOG_NUMBER>(CAOS_LOG_NUMBER_LIMIT_MIN), CAOS_LOG_NUMBER_ERRMSG);
// -------------------------------------------------------------------------------------------------



// CAOS_LOG_TERMINAL_PATTERN -----------------------------------------------------------------------
#ifndef CAOS_LOG_TERMINAL_PATTERN
  #define CAOS_LOG_TERMINAL_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n:%P] %v"
#endif

#define CAOS_LOG_TERMINAL_PATTERN_ERRMSG "CAOS_LOG_TERMINAL_PATTERN" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_LOG_TERMINAL_PATTERN), CAOS_LOG_TERMINAL_PATTERN_ERRMSG);
// -------------------------------------------------------------------------------------------------



// CAOS_LOG_ROTATING_PATTERN -----------------------------------------------------------------------
#ifndef CAOS_LOG_ROTATING_PATTERN
  #define CAOS_LOG_ROTATING_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
#endif

#define CAOS_LOG_ROTATING_PATTERN_ERRMSG "CAOS_LOG_ROTATING_PATTERN" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_LOG_ROTATING_PATTERN), CAOS_LOG_ROTATING_PATTERN_ERRMSG);
// -------------------------------------------------------------------------------------------------



// CAOS_LOG_SYSLOG_PATTERN -------------------------------------------------------------------------
#ifndef CAOS_LOG_SYSLOG_PATTERN
  #define CAOS_LOG_SYSLOG_PATTERN "[%l] %v"
#endif

#define CAOS_LOG_SYSLOG_PATTERN_ERRMSG "CAOS_LOG_SYSLOG_PATTERN" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_LOG_SYSLOG_PATTERN), CAOS_LOG_SYSLOG_PATTERN_ERRMSG);
//--------------------------------------------------------------------------------------------------





// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CROWCPP
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CAOS_USE_CROWCPP

  // Definitions



  // CAOS_CROWCPP_HOST_ENV_NAME --------------------------------------------------------------------
  #ifndef CAOS_CROWCPP_HOST_ENV_NAME
    #define CAOS_CROWCPP_HOST_ENV_NAME "CAOS_CROWCPP_HOST"
  #endif

  #define CAOS_CROWCPP_HOST_ENV_NAME_ERRMSG "CAOS_CROWCPP_HOST_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_HOST_ENV_NAME), CAOS_CROWCPP_HOST_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CROWCPP_HOST_OPT_NAME --------------------------------------------------------------------
  #ifndef CAOS_CROWCPP_HOST_OPT_NAME
    #define CAOS_CROWCPP_HOST_OPT_NAME "crowcpp_host"
  #endif

  #define CAOS_CROWCPP_HOST_OPT_NAME_ERRMSG "CAOS_CROWCPP_HOST_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_HOST_OPT_NAME), CAOS_CROWCPP_HOST_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CROWCPP_PORT_ENV_NAME --------------------------------------------------------------------
  #ifndef CAOS_CROWCPP_PORT_ENV_NAME
    #define CAOS_CROWCPP_PORT_ENV_NAME "CAOS_CROWCPP_PORT"
  #endif

  #define CAOS_CROWCPP_PORT_ENV_NAME_ERRMSG "CAOS_CROWCPP_PORT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_PORT_ENV_NAME), CAOS_CROWCPP_PORT_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CROWCPP_PORT_OPT_NAME --------------------------------------------------------------------
  #ifndef CAOS_CROWCPP_PORT_OPT_NAME
    #define CAOS_CROWCPP_PORT_OPT_NAME "crowcpp_port"
  #endif

  #define CAOS_CROWCPP_PORT_OPT_NAME_ERRMSG "CAOS_CROWCPP_PORT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_PORT_OPT_NAME), CAOS_CROWCPP_PORT_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CROWCPP_THREADS_ENV_NAME -----------------------------------------------------------------
  #ifndef CAOS_CROWCPP_THREADS_ENV_NAME
    #define CAOS_CROWCPP_THREADS_ENV_NAME "CAOS_THREADS"
  #endif

  #define CAOS_CROWCPP_THREADS_ENV_NAME_ERRMSG "CAOS_CROWCPP_THREADS_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_THREADS_ENV_NAME), CAOS_CROWCPP_THREADS_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CROWCPP_THREADS_OPT_NAME -----------------------------------------------------------------
  #ifndef CAOS_CROWCPP_THREADS_OPT_NAME
    #define CAOS_CROWCPP_THREADS_OPT_NAME "threads"
  #endif

  #define CAOS_CROWCPP_THREADS_OPT_NAME_ERRMSG "CAOS_CROWCPP_THREADS_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_THREADS_OPT_NAME), CAOS_CROWCPP_THREADS_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Default values



  // Crowcpp bind to address -----------------------------------------------------------------------
  #define CAOS_CROWCPP_HOST_DEFAULT "127.0.0.1"

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CROWCPP_HOST_ALT
      #undef CAOS_CROWCPP_HOST
      #define CAOS_CROWCPP_HOST CAOS_CROWCPP_HOST_ALT
    #endif
  #endif

  #ifndef CAOS_CROWCPP_HOST
    #define CAOS_CROWCPP_HOST CAOS_CROWCPP_HOST_DEFAULT
  #endif

  #define CAOS_CROWCPP_HOST_ERRMSG "CAOS_CROWCPP_HOST" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CROWCPP_HOST), CAOS_CROWCPP_HOST_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Crowcpp bind to port --------------------------------------------------------------------------
  #define CAOS_CROWCPP_PORT_DEFAULT 18080

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CROWCPP_PORT_ALT
      #undef CAOS_CROWCPP_PORT
      #define CAOS_CROWCPP_PORT CAOS_CROWCPP_PORT_ALT
    #endif
  #endif

  #ifndef CAOS_CROWCPP_PORT
    #define CAOS_CROWCPP_PORT CAOS_CROWCPP_PORT_DEFAULT
  #endif

  #define CAOS_CROWCPP_PORT_ERRMSG "CAOS_CROWCPP_PORT" APPEND_ERRMSG_OUT_OF_RANGE
  static_assert(is_in_range(DEFAULT_UNPRIVILEGED_PORT_MIN, DEFAULT_UNPRIVILEGED_PORT_MAX, CAOS_CROWCPP_PORT), CAOS_CROWCPP_PORT_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Crowcpp threads -------------------------------------------------------------------------------
  #define CAOS_CROWCPP_THREADS_LIMIT_MIN  1
  #define CAOS_CROWCPP_THREADS_DEFAULT    CAOS_DEFAULT_THREADS_VALUE

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CROWCPP_THREADS_ALT
      #undef CAOS_CROWCPP_THREADS
      #define CAOS_CROWCPP_THREADS CAOS_CROWCPP_THREADS_ALT
    #endif
  #endif

  #ifndef CAOS_CROWCPP_THREADS
    #define CAOS_CROWCPP_THREADS CAOS_CROWCPP_THREADS_DEFAULT
  #endif

  #define CAOS_CROWCPP_THREADS_ERRMSG "CAOS_CROWCPP_THREADS" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CROWCPP_THREADS_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CROWCPP_THREADS>(CAOS_CROWCPP_THREADS_LIMIT_MIN), CAOS_CROWCPP_THREADS_ERRMSG);
  //------------------------------------------------------------------------------------------------



#endif
// -------------------------------------------------------------------------------------------------
// End Of CAOS_USE_CROWCPP
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------





// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Cache
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CAOS_USE_CACHE

  // Definitions



  // CAOS_CACHEUSER_ENV_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEUSER_ENV_NAME
    #define CAOS_CACHEUSER_ENV_NAME "CAOS_CACHEUSER"
  #endif

  #define CAOS_CACHEUSER_ENV_NAME_ERRMSG "CAOS_CACHEUSER_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEUSER_ENV_NAME), CAOS_CACHEUSER_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPASS_ENV_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEPASS_ENV_NAME
    #define CAOS_CACHEPASS_ENV_NAME "CAOS_CACHEPASS"
  #endif

  #define CAOS_CACHEPASS_ENV_NAME_ERRMSG "CAOS_CACHEPASS_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPASS_ENV_NAME), CAOS_CACHEPASS_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEHOST_ENV_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEHOST_ENV_NAME
    #define CAOS_CACHEHOST_ENV_NAME "CAOS_CACHEHOST"
  #endif

  #define CAOS_CACHEHOST_ENV_NAME_ERRMSG "CAOS_CACHEHOST_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEHOST_ENV_NAME), CAOS_CACHEHOST_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------




  // CAOS_CACHEPORT_ENV_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEPORT_ENV_NAME
    #define CAOS_CACHEPORT_ENV_NAME "CAOS_CACHEPORT"
  #endif

  #define CAOS_CACHEPORT_ENV_NAME_ERRMSG "CAOS_CACHEPORT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPORT_ENV_NAME), CAOS_CACHEPORT_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHECLIENTNAME_ENV_NAME -----------------------------------------------------------------
  #ifndef CAOS_CACHECLIENTNAME_ENV_NAME
    #define CAOS_CACHECLIENTNAME_ENV_NAME "CAOS_CACHECLIENTNAME"
  #endif

  #define CAOS_CACHECLIENTNAME_ENV_NAME_ERRMSG "CAOS_CACHECLIENTNAME_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHECLIENTNAME_ENV_NAME), CAOS_CACHECLIENTNAME_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEINDEX_ENV_NAME ----------------------------------------------------------------------
  #ifndef CAOS_CACHEINDEX_ENV_NAME
    #define CAOS_CACHEINDEX_ENV_NAME "CAOS_CACHEINDEX"
  #endif

  #define CAOS_CACHEINDEX_ENV_NAME_ERRMSG "CAOS_CACHEINDEX_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEINDEX_ENV_NAME), CAOS_CACHEINDEX_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHECOMMANDTIMEOUT_ENV_NAME -------------------------------------------------------------
  #ifndef CAOS_CACHECOMMANDTIMEOUT_ENV_NAME
    #define CAOS_CACHECOMMANDTIMEOUT_ENV_NAME "CAOS_CACHECOMMANDTIMEOUT"
  #endif

  #define CAOS_CACHECOMMANDTIMEOUT_ENV_NAME_ERRMSG "CAOS_CACHECOMMANDTIMEOUT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHECOMMANDTIMEOUT_ENV_NAME), CAOS_CACHECOMMANDTIMEOUT_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLSIZEMIN_ENV_NAME ----------------------------------------------------------------
  #ifndef CAOS_CACHEPOOLSIZEMIN_ENV_NAME
    #define CAOS_CACHEPOOLSIZEMIN_ENV_NAME "CAOS_CACHEPOOLSIZEMIN"
  #endif

  #define CAOS_CACHEPOOLSIZEMIN_ENV_NAME_ERRMSG "CAOS_CACHEPOOLSIZEMIN_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLSIZEMIN_ENV_NAME), CAOS_CACHEPOOLSIZEMIN_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLSIZEMAX_ENV_NAME ----------------------------------------------------------------
  #ifndef CAOS_CACHEPOOLSIZEMAX_ENV_NAME
    #define CAOS_CACHEPOOLSIZEMAX_ENV_NAME "CAOS_CACHEPOOLSIZEMAX"
  #endif

  #define CAOS_CACHEPOOLSIZEMAX_ENV_NAME_ERRMSG "CAOS_CACHEPOOLSIZEMAX_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLSIZEMAX_ENV_NAME), CAOS_CACHEPOOLSIZEMAX_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLWAIT_ENV_NAME -------------------------------------------------------------------
  #ifndef CAOS_CACHEPOOLWAIT_ENV_NAME
    #define CAOS_CACHEPOOLWAIT_ENV_NAME "CAOS_CACHEPOOLWAIT"
  #endif

  #define CAOS_CACHEPOOLWAIT_ENV_NAME_ERRMSG "CAOS_CACHEPOOLWAIT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLWAIT_ENV_NAME), CAOS_CACHEPOOLWAIT_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME ------------------------------------------------------
  #ifndef CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME
    #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME "CAOS_CACHEPOOLCONNECTIONTIMEOUT"
  #endif

  #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME_ERRMSG "CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME), CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME -----------------------------------------------------
  #ifndef CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME
    #define CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME "CAOS_CACHEPOOLCONNECTIONLIFETIME"
  #endif

  #define CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME_ERRMSG "CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME), CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME -----------------------------------------------------
  #ifndef CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME
    #define CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME "CAOS_CACHEPOOLCONNECTIONIDLETIME"
  #endif

  #define CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME_ERRMSG "CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME), CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------





  // CAOS_CACHEUSER_OPT_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEUSER_OPT_NAME
    #define CAOS_CACHEUSER_OPT_NAME "cacheuser"
  #endif

  #define CAOS_CACHEUSER_OPT_NAME_ERRMSG "CAOS_CACHEUSER_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEUSER_OPT_NAME), CAOS_CACHEUSER_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPASS_OPT_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEPASS_OPT_NAME
    #define CAOS_CACHEPASS_OPT_NAME "cachepass"
  #endif

  #define CAOS_CACHEPASS_OPT_NAME_ERRMSG "CAOS_CACHEPASS_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPASS_OPT_NAME), CAOS_CACHEPASS_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEHOST_OPT_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEHOST_OPT_NAME
    #define CAOS_CACHEHOST_OPT_NAME "cachehost"
  #endif

  #define CAOS_CACHEHOST_OPT_NAME_ERRMSG "CAOS_CACHEHOST_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEHOST_OPT_NAME), CAOS_CACHEHOST_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------




  // CAOS_CACHEPORT_OPT_NAME -----------------------------------------------------------------------
  #ifndef CAOS_CACHEPORT_OPT_NAME
    #define CAOS_CACHEPORT_OPT_NAME "cacheport"
  #endif

  #define CAOS_CACHEPORT_OPT_NAME_ERRMSG "CAOS_CACHEPORT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPORT_OPT_NAME), CAOS_CACHEPORT_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHECLIENTNAME_OPT_NAME -----------------------------------------------------------------
  #ifndef CAOS_CACHECLIENTNAME_OPT_NAME
    #define CAOS_CACHECLIENTNAME_OPT_NAME "cacheclientname"
  #endif

  #define CAOS_CACHECLIENTNAME_OPT_NAME_ERRMSG "CAOS_CACHECLIENTNAME_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHECLIENTNAME_OPT_NAME), CAOS_CACHECLIENTNAME_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEINDEX_OPT_NAME ----------------------------------------------------------------------
  #ifndef CAOS_CACHEINDEX_OPT_NAME
    #define CAOS_CACHEINDEX_OPT_NAME "cacheindex"
  #endif

  #define CAOS_CACHEINDEX_OPT_NAME_ERRMSG "CAOS_CACHEINDEX_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEINDEX_OPT_NAME), CAOS_CACHEINDEX_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHECOMMANDTIMEOUT_OPT_NAME -------------------------------------------------------------
  #ifndef CAOS_CACHECOMMANDTIMEOUT_OPT_NAME
    #define CAOS_CACHECOMMANDTIMEOUT_OPT_NAME "cachecommandtimeout"
  #endif

  #define CAOS_CACHECOMMANDTIMEOUT_OPT_NAME_ERRMSG "CAOS_CACHECOMMANDTIMEOUT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHECOMMANDTIMEOUT_OPT_NAME), CAOS_CACHECOMMANDTIMEOUT_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLSIZEMIN_OPT_NAME ----------------------------------------------------------------
  #ifndef CAOS_CACHEPOOLSIZEMIN_OPT_NAME
    #define CAOS_CACHEPOOLSIZEMIN_OPT_NAME "cachepoolsizemin"
  #endif

  #define CAOS_CACHEPOOLSIZEMIN_OPT_NAME_ERRMSG "CAOS_CACHEPOOLSIZEMIN_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLSIZEMIN_OPT_NAME), CAOS_CACHEPOOLSIZEMIN_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLSIZEMAX_OPT_NAME ----------------------------------------------------------------
  #ifndef CAOS_CACHEPOOLSIZEMAX_OPT_NAME
    #define CAOS_CACHEPOOLSIZEMAX_OPT_NAME "cachepoolsizemax"
  #endif

  #define CAOS_CACHEPOOLSIZEMAX_OPT_NAME_ERRMSG "CAOS_CACHEPOOLSIZEMAX_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLSIZEMAX_OPT_NAME), CAOS_CACHEPOOLSIZEMAX_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLWAIT_OPT_NAME -------------------------------------------------------------------
  #ifndef CAOS_CACHEPOOLWAIT_OPT_NAME
    #define CAOS_CACHEPOOLWAIT_OPT_NAME "CACHEPOOLWAIT"
  #endif

  #define CAOS_CACHEPOOLWAIT_OPT_NAME_ERRMSG "CAOS_CACHEPOOLWAIT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLWAIT_OPT_NAME), CAOS_CACHEPOOLWAIT_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_POOLCONNECTIONTIMEOUT_OPT_NAME -----------------------------------------------------------
  #ifndef CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME
    #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME "cachepoolconnectiontimeout"
  #endif

  #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME_ERRMSG "CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME), CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME -----------------------------------------------------
  #ifndef CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME
    #define CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME "cachepoolconnectionlifetime"
  #endif

  #define CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME_ERRMSG "CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME), CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME -----------------------------------------------------
  #ifndef CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME
    #define CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME "cachepoolconnectionidletime"
  #endif

  #define CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME_ERRMSG "CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME), CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Default values



  // Cache User ------------------------------------------------------------------------------------
  #define CAOS_CACHEUSER_DEFAULT ""

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #undef CAOS_CACHEUSER

    #ifdef CAOS_CACHEUSER_ALT
      #define CAOS_CACHEUSER CAOS_CACHEUSER_ALT
    #else
      #define CAOS_CACHEUSER CAOS_CACHEUSER_DEFAULT
    #endif
  #endif

  #ifndef CAOS_CACHEUSER
    #define CAOS_CACHEUSER CAOS_CACHEUSER_DEFAULT
  #endif

  #define CAOS_CACHEUSER_ERRMSG "CAOS_CACHEUSER" APPEND_ERRMSG_NON_NULL
  static_assert(is_non_null_string(CAOS_CACHEUSER), CAOS_CACHEUSER_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache Password --------------------------------------------------------------------------------
  #define CAOS_CACHEPASS_DEFAULT ""

  #ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
    #undef CAOS_CACHEPASS

    #ifdef CAOS_CACHEPASS_ALT
      #define CAOS_CACHEPASS CAOS_CACHEPASS_ALT
    #else
      #define CAOS_CACHEPASS CAOS_CACHEPASS_DEFAULT
    #endif
  #endif

  #ifndef CAOS_CACHEPASS
    #define CAOS_CACHEPASS CAOS_CACHEPASS_DEFAULT
  #endif

  #define CAOS_CACHEPASS_ERRMSG "CAOS_CACHEPASS" APPEND_ERRMSG_NON_NULL
  static_assert(is_non_null_string(CAOS_CACHEPASS), CAOS_CACHEPASS_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache Host ------------------------------------------------------------------------------------
  #define CAOS_CACHEHOST_DEFAULT "127.0.0.1"

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEHOST_ALT
      #undef CAOS_CACHEHOST
      #define CAOS_CACHEHOST CAOS_CACHEHOST_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEHOST
    #define CAOS_CACHEHOST CAOS_CACHEHOST_DEFAULT
  #endif

  #define CAOS_CACHEHOST_ERRMSG "CAOS_CACHEHOST" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_CACHEHOST), CAOS_CACHEHOST_ERRMSG);
  //------------------------------------------------------------------------------------------------




  // Cache Port ------------------------------------------------------------------------------------
  #ifdef CAOS_USE_CACHE_REDIS
    #define CAOS_CACHEPORT_DEFAULT 6379
  #else
    #error "Cache port default configuration unknown"
  #endif


  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPORT_ALT
      #undef CAOS_CACHEPORT
      #define CAOS_CACHEPORT CAOS_CACHEPORT_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPORT
    #define CAOS_CACHEPORT CAOS_CACHEPORT_DEFAULT
  #endif

  #define CAOS_CACHEPORT_ERRMSG "CAOS_CACHEPORT" APPEND_ERRMSG_OUT_OF_RANGE
  static_assert(is_in_range(DEFAULT_UNPRIVILEGED_PORT_MIN, DEFAULT_UNPRIVILEGED_PORT_MAX, CAOS_CACHEPORT), CAOS_CACHEPORT_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache Client Name -----------------------------------------------------------------------------
  #define CAOS_CACHECLIENTNAME_DEFAULT "CAOS"

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHECLIENTNAME_ALT
      #undef CAOS_CACHECLIENTNAME
      #define CAOS_CACHECLIENTNAME CAOS_CACHECLIENTNAME_ALT
    #endif
  #endif

  #ifndef CAOS_CACHECLIENTNAME
    #define CAOS_CACHECLIENTNAME CAOS_CACHECLIENTNAME_DEFAULT
  #endif

  #define CAOS_CACHECLIENTNAME_ERRMSG "CAOS_CACHECLIENTNAME" APPEND_ERRMSG_NON_NULL
  static_assert(is_non_null_string(CAOS_CACHECLIENTNAME), CAOS_CACHECLIENTNAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache Index -----------------------------------------------------------------------------------
  #define CAOS_CACHEINDEX_DEFAULT 0
  #define CAOS_CACHEINDEX_LIMIT_MIN 0
  #define CAOS_CACHEINDEX_LIMIT_MAX 15

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEINDEX_ALT
      #undef CAOS_CACHEINDEX
      #define CAOS_CACHEINDEX CAOS_CACHEINDEX_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEINDEX
    #define CAOS_CACHEINDEX CAOS_CACHEINDEX_DEFAULT
  #endif

  #define CAOS_CACHEINDEX_ERRMSG "CAOS_CACHEINDEX" APPEND_ERRMSG_OUT_OF_RANGE
  static_assert(is_in_range(CAOS_CACHEINDEX_LIMIT_MIN, CAOS_CACHEINDEX_LIMIT_MAX, CAOS_CACHEINDEX), CAOS_CACHEINDEX_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache command timeout -------------------------------------------------------------------------
  #define CAOS_CACHECOMMANDTIMEOUT_DEFAULT    0
  #define CAOS_CACHECOMMANDTIMEOUT_LIMIT_MIN  0

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHECOMMANDTIMEOUT_ALT
      #undef CAOS_CACHECOMMANDTIMEOUT
      #define CAOS_CACHECOMMANDTIMEOUT CAOS_CACHECOMMANDTIMEOUT_ALT
    #endif
  #endif

  #ifndef CAOS_CACHECOMMANDTIMEOUT
    #define CAOS_CACHECOMMANDTIMEOUT CAOS_CACHECOMMANDTIMEOUT_DEFAULT
  #endif

  #define CAOS_CACHECOMMANDTIMEOUT_ERRMSG "CAOS_CACHECOMMANDTIMEOUT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHECOMMANDTIMEOUT_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHECOMMANDTIMEOUT>(CAOS_CACHECOMMANDTIMEOUT_LIMIT_MIN), CAOS_CACHECOMMANDTIMEOUT_ERRMSG);
  //------------------------------------------------------------------------------------------------


  // Cache poolsize min ----------------------------------------------------------------------------
  #define CAOS_CACHEPOOLSIZEMIN_DEFAULT 10
  #define CAOS_CACHEPOOLSIZEMIN_LIMIT_MIN 1

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPOOLSIZEMIN_ALT
      #undef CAOS_CACHEPOOLSIZEMIN
      #define CAOS_CACHEPOOLSIZEMIN CAOS_CACHEPOOLSIZEMIN_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPOOLSIZEMIN
    #define CAOS_CACHEPOOLSIZEMIN CAOS_CACHEPOOLSIZEMIN_DEFAULT
  #endif

  #define CAOS_CACHEPOOLSIZEMIN_ERRMSG "CAOS_CACHEPOOLSIZEMIN" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHEPOOLSIZEMIN_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHEPOOLSIZEMIN>(CAOS_CACHEPOOLSIZEMIN_LIMIT_MIN), CAOS_CACHEPOOLSIZEMIN_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache poolsize max ----------------------------------------------------------------------------
  #define CAOS_CACHEPOOLSIZEMAX_DEFAULT 20
  #define CAOS_CACHEPOOLSIZEMAX_LIMIT_MIN 1

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPOOLSIZEMAX_ALT
      #undef CAOS_CACHEPOOLSIZEMAX
      #define CAOS_CACHEPOOLSIZEMAX CAOS_CACHEPOOLSIZEMAX_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPOOLSIZEMAX
    #define CAOS_CACHEPOOLSIZEMAX CAOS_CACHEPOOLSIZEMAX_DEFAULT
  #endif

  #define CAOS_CACHEPOOLSIZEMAX_ERRMSG "CAOS_CACHEPOOLSIZEMAX" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHEPOOLSIZEMAX_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHEPOOLSIZEMAX>(CAOS_CACHEPOOLSIZEMAX_LIMIT_MIN), CAOS_CACHEPOOLSIZEMAX_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Check if poolsize min < poolsize max ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  #if CAOS_CACHEPOOLSIZEMIN > CAOS_CACHEPOOLSIZEMAX
    #error "CAOS_CACHEPOOLSIZEMIN > CAOS_CACHEPOOLSIZEMAX"
  #endif
  // Check if poolsize min < poolsize max ------------------------------------------------------------



  // Cache poolwait ----------------------------------------------------------------------------
  #define CAOS_CACHEPOOLWAIT_DEFAULT 100
  #define CAOS_CACHEPOOLWAIT_LIMIT_MIN 1

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPOOLWAIT_ALT
      #undef CAOS_CACHEPOOLWAIT
      #define CAOS_CACHEPOOLWAIT CAOS_CACHEPOOLWAIT_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPOOLWAIT
    #define CAOS_CACHEPOOLWAIT CAOS_CACHEPOOLWAIT_DEFAULT
  #endif

  #define CAOS_CACHEPOOLWAIT_ERRMSG "CAOS_CACHEPOOLWAIT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHEPOOLWAIT_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHEPOOLWAIT>(CAOS_CACHEPOOLWAIT_LIMIT_MIN), CAOS_CACHEPOOLWAIT_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache connection timeout ----------------------------------------------------------------------
  #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_DEFAULT 100
  #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_LIMIT_MIN 1

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPOOLCONNECTIONTIMEOUT_ALT
      #undef CAOS_CACHEPOOLCONNECTIONTIMEOUT
      #define CAOS_CACHEPOOLCONNECTIONTIMEOUT CAOS_CACHEPOOLCONNECTIONTIMEOUT_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPOOLCONNECTIONTIMEOUT
    #define CAOS_CACHEPOOLCONNECTIONTIMEOUT CAOS_CACHEPOOLCONNECTIONTIMEOUT_DEFAULT
  #endif

  #define CAOS_CACHEPOOLCONNECTIONTIMEOUT_ERRMSG "CAOS_CACHEPOOLCONNECTIONTIMEOUT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHEPOOLCONNECTIONTIMEOUT_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHEPOOLCONNECTIONTIMEOUT>(CAOS_CACHEPOOLCONNECTIONTIMEOUT_LIMIT_MIN), CAOS_CACHEPOOLCONNECTIONTIMEOUT_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache connection timeout ----------------------------------------------------------------------
  #define CAOS_CACHEPOOLCONNECTIONLIFETIME_DEFAULT 100
  #define CAOS_CACHEPOOLCONNECTIONLIFETIME_LIMIT_MIN 1

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPOOLCONNECTIONLIFETIME_ALT
      #undef CAOS_CACHEPOOLCONNECTIONLIFETIME
      #define CAOS_CACHEPOOLCONNECTIONLIFETIME CAOS_CACHEPOOLCONNECTIONLIFETIME_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPOOLCONNECTIONLIFETIME
    #define CAOS_CACHEPOOLCONNECTIONLIFETIME CAOS_CACHEPOOLCONNECTIONLIFETIME_DEFAULT
  #endif

  #define CAOS_CACHEPOOLCONNECTIONLIFETIME_ERRMSG "CAOS_CACHEPOOLCONNECTIONLIFETIME" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHEPOOLCONNECTIONLIFETIME_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHEPOOLCONNECTIONLIFETIME>(CAOS_CACHEPOOLCONNECTIONLIFETIME_LIMIT_MIN), CAOS_CACHEPOOLCONNECTIONLIFETIME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Cache connection timeout ----------------------------------------------------------------------
  #define CAOS_CACHEPOOLCONNECTIONIDLETIME_DEFAULT 100
  #define CAOS_CACHEPOOLCONNECTIONIDLETIME_LIMIT_MIN 1

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_CACHEPOOLCONNECTIONIDLETIME_ALT
      #undef CAOS_CACHEPOOLCONNECTIONIDLETIME
      #define CAOS_CACHEPOOLCONNECTIONIDLETIME CAOS_CACHEPOOLCONNECTIONIDLETIME_ALT
    #endif
  #endif

  #ifndef CAOS_CACHEPOOLCONNECTIONIDLETIME
    #define CAOS_CACHEPOOLCONNECTIONIDLETIME CAOS_CACHEPOOLCONNECTIONIDLETIME_DEFAULT
  #endif

  #define CAOS_CACHEPOOLCONNECTIONIDLETIME_ERRMSG "CAOS_CACHEPOOLCONNECTIONIDLETIME" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_CACHEPOOLCONNECTIONIDLETIME_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_CACHEPOOLCONNECTIONIDLETIME>(CAOS_CACHEPOOLCONNECTIONIDLETIME_LIMIT_MIN), CAOS_CACHEPOOLCONNECTIONIDLETIME_ERRMSG);
  //------------------------------------------------------------------------------------------------

#endif
//--------------------------------------------------------------------------------------------------
// End Of CAOS_USE_CACHE
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Database
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Definitions



// CAOS_DBUSER_ENV_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBUSER_ENV_NAME
  #define CAOS_DBUSER_ENV_NAME "CAOS_DBUSER"
#endif

#define CAOS_DBUSER_ENV_NAME_ERRMSG "CAOS_DBUSER_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBUSER_ENV_NAME), CAOS_DBUSER_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPASS_ENV_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBPASS_ENV_NAME
  #define CAOS_DBPASS_ENV_NAME "CAOS_DBPASS"
#endif

#define CAOS_DBPASS_ENV_NAME_ERRMSG "CAOS_DBPASS_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPASS_ENV_NAME), CAOS_DBPASS_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBHOST_ENV_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBHOST_ENV_NAME
  #define CAOS_DBHOST_ENV_NAME "CAOS_DBHOST"
#endif

#define CAOS_DBHOST_ENV_NAME_ERRMSG "CAOS_DBHOST_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBHOST_ENV_NAME), CAOS_DBHOST_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------




// CAOS_DBPORT_ENV_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBPORT_ENV_NAME
  #define CAOS_DBPORT_ENV_NAME "CAOS_DBPORT"
#endif

#define CAOS_DBPORT_ENV_NAME_ERRMSG "CAOS_DBPORT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPORT_ENV_NAME), CAOS_DBPORT_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------


// CAOS_DBNAME_ENV_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBNAME_ENV_NAME
  #define CAOS_DBNAME_ENV_NAME "CAOS_DBNAME"
#endif

#define CAOS_DBNAME_ENV_NAME_ERRMSG "CAOS_DBNAME_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBNAME_ENV_NAME), CAOS_DBNAME_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPOOLSIZEMIN_ENV_NAME ---------------------------------------------------------------------
#ifndef CAOS_DBPOOLSIZEMIN_ENV_NAME
  #define CAOS_DBPOOLSIZEMIN_ENV_NAME "CAOS_DBPOOLSIZEMIN"
#endif

#define CAOS_DBPOOLSIZEMIN_ENV_NAME_ERRMSG "CAOS_DBPOOLSIZEMIN_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLSIZEMIN_ENV_NAME), CAOS_DBPOOLSIZEMIN_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPOOLSIZEMAX_ENV_NAME ---------------------------------------------------------------------
#ifndef CAOS_DBPOOLSIZEMAX_ENV_NAME
  #define CAOS_DBPOOLSIZEMAX_ENV_NAME "CAOS_DBPOOLSIZEMAX"
#endif

#define CAOS_DBPOOLSIZEMAX_ENV_NAME_ERRMSG "CAOS_DBPOOLSIZEMAX_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLSIZEMAX_ENV_NAME), CAOS_DBPOOLSIZEMAX_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPOOLWAIT_ENV_NAME ------------------------------------------------------------------------
#ifndef CAOS_DBPOOLWAIT_ENV_NAME
  #define CAOS_DBPOOLWAIT_ENV_NAME "CAOS_DBPOOLWAIT"
#endif

#define CAOS_DBPOOLWAIT_ENV_NAME_ERRMSG "CAOS_DBPOOLWAIT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLWAIT_ENV_NAME), CAOS_DBPOOLWAIT_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------




// CAOS_DBPOOLTIMEOUT_ENV_NAME ---------------------------------------------------------------------
#ifndef CAOS_DBPOOLTIMEOUT_ENV_NAME
  #define CAOS_DBPOOLTIMEOUT_ENV_NAME "CAOS_DBPOOLTIMEOUT"
#endif

#define CAOS_DBPOOLTIMEOUT_ENV_NAME_ERRMSG "CAOS_DBPOOLTIMEOUT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLTIMEOUT_ENV_NAME), CAOS_DBPOOLTIMEOUT_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------


#ifdef CAOS_USE_DB_POSTGRESQL

  // CAOS_DBKEEPALIVES_ENV_NAME -------------------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_ENV_NAME
    #define CAOS_DBKEEPALIVES_ENV_NAME "CAOS_DBKEEPALIVES"
  #endif

  #define CAOS_DBKEEPALIVES_ENV_NAME_ERRMSG "CAOS_DBKEEPALIVES_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_ENV_NAME), CAOS_DBKEEPALIVES_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_DBKEEPALIVES_IDLE_ENV_NAME ---------------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_IDLE_ENV_NAME
    #define CAOS_DBKEEPALIVES_IDLE_ENV_NAME "CAOS_DBKEEPALIVES_IDLE"
  #endif

  #define CAOS_DBKEEPALIVES_IDLE_ENV_NAME_ERRMSG "CAOS_DBKEEPALIVES_IDLE_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_IDLE_ENV_NAME), CAOS_DBKEEPALIVES_IDLE_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME -----------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME
    #define CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME "CAOS_DBKEEPALIVES_INTERVAL"
  #endif

  #define CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME_ERRMSG "CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME), CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_DBKEEPALIVES_COUNT_ENV_NAME --------------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_COUNT_ENV_NAME
    #define CAOS_DBKEEPALIVES_COUNT_ENV_NAME "CAOS_DBKEEPALIVES_COUNT"
  #endif

  #define CAOS_DBKEEPALIVES_COUNT_ENV_NAME_ERRMSG "CAOS_DBKEEPALIVES_COUNT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_COUNT_ENV_NAME), CAOS_DBKEEPALIVES_COUNT_ENV_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------

#endif // End Of CAOS_USE_DB_POSTGRESQL

// CAOS_DBCONNECT_TIMEOUT_ENV_NAME -----------------------------------------------------------------
#ifndef CAOS_DBCONNECT_TIMEOUT_ENV_NAME
  #define CAOS_DBCONNECT_TIMEOUT_ENV_NAME "CAOS_DBCONNECT_TIMEOUT"
#endif

#define CAOS_DBCONNECT_TIMEOUT_ENV_NAME_ERRMSG "CAOS_DBCONNECT_TIMEOUT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBCONNECT_TIMEOUT_ENV_NAME), CAOS_DBCONNECT_TIMEOUT_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBCONNECT_TIMEOUT_ENV_NAME -----------------------------------------------------------------
#ifndef CAOS_DBMAXWAIT_ENV_NAME
  #define CAOS_DBMAXWAIT_ENV_NAME "CAOS_DBMAXWAIT"
#endif

#define CAOS_DBMAXWAIT_ENV_NAME_ERRMSG "CAOS_DBMAXWAIT_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBMAXWAIT_ENV_NAME), CAOS_DBMAXWAIT_ENV_NAME_ERRMSG);
//------------------------------------------------------------------------------------------------



// CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME -------------------------------------------------------------
#ifndef CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME
  #define CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME "CAOS_DBHEALTHCHECKINTERVAL"
#endif

#define CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME_ERRMSG "CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME), CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME -------------------------------------------
#ifndef CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME
  #define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME "CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED"
#endif

#define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME_ERRMSG "CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME), CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME ------------------------------------------------
#ifndef CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME
  #define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME "CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE"
#endif

#define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME_ERRMSG "CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME), CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME --------------------------------------------------------
#ifndef CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME
  #define CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME "CAOS_VALIDATE_USING_TRANSACTION"
#endif

#define CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME_ERRMSG "CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME), CAOS_VALIDATE_USING_TRANSACTION_ENV_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBUSER_OPT_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBUSER_OPT_NAME
  #define CAOS_DBUSER_OPT_NAME "dbuser"
#endif

#define CAOS_DBUSER_OPT_NAME_ERRMSG "CAOS_DBUSER_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBUSER_OPT_NAME), CAOS_DBUSER_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPASS_OPT_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBPASS_OPT_NAME
  #define CAOS_DBPASS_OPT_NAME "dbpass"
#endif

#define CAOS_DBPASS_OPT_NAME_ERRMSG "CAOS_DBPASS_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPASS_OPT_NAME), CAOS_DBPASS_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBHOST_OPT_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBHOST_OPT_NAME
  #define CAOS_DBHOST_OPT_NAME "dbhost"
#endif

#define CAOS_DBHOST_OPT_NAME_ERRMSG "CAOS_DBHOST_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBHOST_OPT_NAME), CAOS_DBHOST_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------




// CAOS_DBPORT_OPT_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBPORT_OPT_NAME
  #define CAOS_DBPORT_OPT_NAME "dbport"
#endif

#define CAOS_DBPORT_OPT_NAME_ERRMSG "CAOS_DBPORT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPORT_OPT_NAME), CAOS_DBPORT_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------


// CAOS_DBNAME_OPT_NAME ----------------------------------------------------------------------------
#ifndef CAOS_DBNAME_OPT_NAME
  #define CAOS_DBNAME_OPT_NAME "dbname"
#endif

#define CAOS_DBNAME_OPT_NAME_ERRMSG "CAOS_DBNAME_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBNAME_OPT_NAME), CAOS_DBNAME_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPOOLSIZEMIN_OPT_NAME ---------------------------------------------------------------------
#ifndef CAOS_DBPOOLSIZEMIN_OPT_NAME
  #define CAOS_DBPOOLSIZEMIN_OPT_NAME "dbpoolsizemin"
#endif

#define CAOS_DBPOOLSIZEMIN_OPT_NAME_ERRMSG "CAOS_DBPOOLSIZEMIN_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLSIZEMIN_OPT_NAME), CAOS_DBPOOLSIZEMIN_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPOOLSIZEMAX_OPT_NAME ---------------------------------------------------------------------
#ifndef CAOS_DBPOOLSIZEMAX_OPT_NAME
  #define CAOS_DBPOOLSIZEMAX_OPT_NAME "dbpoolsizemax"
#endif

#define CAOS_DBPOOLSIZEMAX_OPT_NAME_ERRMSG "CAOS_DBPOOLSIZEMAX_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLSIZEMAX_OPT_NAME), CAOS_DBPOOLSIZEMAX_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBPOOLWAIT_OPT_NAME ------------------------------------------------------------------------
#ifndef CAOS_DBPOOLWAIT_OPT_NAME
  #define CAOS_DBPOOLWAIT_OPT_NAME "dbpoolwait"
#endif

#define CAOS_DBPOOLWAIT_OPT_NAME_ERRMSG "CAOS_DBPOOLWAIT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLWAIT_OPT_NAME), CAOS_DBPOOLWAIT_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------




// CAOS_DBPOOLTIMEOUT_OPT_NAME ---------------------------------------------------------------------
#ifndef CAOS_DBPOOLTIMEOUT_OPT_NAME
  #define CAOS_DBPOOLTIMEOUT_OPT_NAME "dbpooltimeout"
#endif

#define CAOS_DBPOOLTIMEOUT_OPT_NAME_ERRMSG "CAOS_DBPOOLTIMEOUT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBPOOLTIMEOUT_OPT_NAME), CAOS_DBPOOLTIMEOUT_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------


#ifdef CAOS_USE_DB_POSTGRESQL

  // CAOS_DBKEEPALIVES_OPT_NAME --------------------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_OPT_NAME
    #define CAOS_DBKEEPALIVES_OPT_NAME "dbkeepalives"
  #endif

  #define CAOS_DBKEEPALIVES_OPT_NAME_ERRMSG "CAOS_DBKEEPALIVES_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_OPT_NAME), CAOS_DBKEEPALIVES_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_DBKEEPALIVES_IDLE_OPT_NAME ---------------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_IDLE_OPT_NAME
    #define CAOS_DBKEEPALIVES_IDLE_OPT_NAME "dbkeepalives_idle"
  #endif

  #define CAOS_DBKEEPALIVES_IDLE_OPT_NAME_ERRMSG "CAOS_DBKEEPALIVES_IDLE_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_IDLE_OPT_NAME), CAOS_DBKEEPALIVES_IDLE_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME -----------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME
    #define CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME "dbkeepalives_interval"
  #endif

  #define CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME_ERRMSG "CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME), CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // CAOS_DBKEEPALIVES_COUNT_OPT_NAME --------------------------------------------------------------
  #ifndef CAOS_DBKEEPALIVES_COUNT_OPT_NAME
    #define CAOS_DBKEEPALIVES_COUNT_OPT_NAME "dbkeepalives_count"
  #endif

  #define CAOS_DBKEEPALIVES_COUNT_OPT_NAME_ERRMSG "CAOS_DBKEEPALIVES_COUNT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
  static_assert(is_non_null_and_non_empty_string(CAOS_DBKEEPALIVES_COUNT_OPT_NAME), CAOS_DBKEEPALIVES_COUNT_OPT_NAME_ERRMSG);
  //------------------------------------------------------------------------------------------------

#endif // End Of CAOS_USE_DB_POSTGRESQL




// CAOS_DBCONNECT_TIMEOUT_OPT_NAME -----------------------------------------------------------------
#ifndef CAOS_DBCONNECT_TIMEOUT_OPT_NAME
  #define CAOS_DBCONNECT_TIMEOUT_OPT_NAME "dbconnect_timeout"
#endif

#define CAOS_DBCONNECT_TIMEOUT_OPT_NAME_ERRMSG "CAOS_DBCONNECT_TIMEOUT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBCONNECT_TIMEOUT_OPT_NAME), CAOS_DBCONNECT_TIMEOUT_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBMAXWAIT_OPT_NAME -------------------------------------------------------------------------
#ifndef CAOS_DBMAXWAIT_OPT_NAME
  #define CAOS_DBMAXWAIT_OPT_NAME "dbmaxwait"
#endif

#define CAOS_DBMAXWAIT_OPT_NAME_ERRMSG "CAOS_DBMAXWAIT_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBMAXWAIT_OPT_NAME), CAOS_DBMAXWAIT_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME -------------------------------------------------------------
#ifndef CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME
  #define CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME "dbhealthcheckinterval"
#endif

#define CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME_ERRMSG "CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME), CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME -------------------------------------------
#ifndef CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME
  #define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME "log_threshold_connection_limit_exceeded"
#endif

#define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME_ERRMSG "CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME), CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME ------------------------------------------------
#ifndef CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME
  #define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME "validate_connection_before_acquire"
#endif

#define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME_ERRMSG "CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME), CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME --------------------------------------------------------
#ifndef CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME
  #define CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME "validate_using_transaction"
#endif

#define CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME_ERRMSG "CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME" APPEND_ERRMSG_NON_EMPTY
static_assert(is_non_null_and_non_empty_string(CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME), CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Default values



// Database User -----------------------------------------------------------------------------------
#define CAOS_DBUSER_DEFAULT     "caos_u"
#define CAOS_DBUSER_DEFAULT_ALT "caos_u_alt"

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #undef CAOS_DBUSER

  #ifdef CAOS_DBUSER_ALT
    #define CAOS_DBUSER CAOS_DBUSER_ALT
  #else
    #define CAOS_DBUSER CAOS_DBUSER_DEFAULT_ALT
  #endif
#endif

#ifndef CAOS_DBUSER
  #define CAOS_DBUSER CAOS_DBUSER_DEFAULT
#endif

#define CAOS_DBUSER_ERRMSG "CAOS_DBUSER" APPEND_ERRMSG_NON_NULL
static_assert(is_non_null_string(CAOS_DBUSER), CAOS_DBUSER_ERRMSG);
//--------------------------------------------------------------------------------------------------




// Database Password -------------------------------------------------------------------------------
#define CAOS_DBPASS_DEFAULT     "verystrongpassword"
#define CAOS_DBPASS_DEFAULT_ALT "verystrongpassword_alt"

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #undef CAOS_DBPASS

  #ifdef CAOS_DBPASS_ALT
    #define CAOS_DBPASS CAOS_DBPASS_ALT
  #else
    #define CAOS_DBPASS CAOS_DBPASS_DEFAULT_ALT
  #endif
#endif

#ifndef CAOS_DBPASS
  #define CAOS_DBPASS CAOS_DBPASS_DEFAULT
#endif

#define CAOS_DBPASS_ERRMSG "CAOS_DBPASS" APPEND_ERRMSG_NON_NULL
static_assert(is_non_null_string(CAOS_DBPASS), CAOS_DBPASS_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database Host -----------------------------------------------------------------------------------
#define CAOS_DBHOST_DEFAULT "127.0.0.1"

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBHOST_ALT
    #undef CAOS_DBHOST
    #define CAOS_DBHOST CAOS_DBHOST_ALT
  #endif
#endif

#ifndef CAOS_DBHOST
  #define CAOS_DBHOST CAOS_DBHOST_DEFAULT
#endif

#define CAOS_DBHOST_ERRMSG "CAOS_DBHOST" APPEND_ERRMSG_NON_NULL
static_assert(is_non_null_string(CAOS_DBHOST), CAOS_DBHOST_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database Port -----------------------------------------------------------------------------------
#ifdef CAOS_USE_DB_POSTGRESQL
  #define CAOS_DBPORT_DEFAULT 5432
#elif (defined(CAOS_USE_DB_MYSQL) || defined(CAOS_USE_DB_MARIADB))
  #define CAOS_DBPORT_DEFAULT 3306
#else
  #error "Database port configuration unknown"
#endif

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBPORT_ALT
    #undef CAOS_DBPORT
    #define CAOS_DBPORT CAOS_DBPORT_ALT
  #endif
#endif

#ifndef CAOS_DBPORT
  #define CAOS_DBPORT CAOS_DBPORT_DEFAULT
#endif

#define CAOS_DBPORT_ERRMSG "CAOS_DBPORT" APPEND_ERRMSG_OUT_OF_RANGE
static_assert(is_in_range(DEFAULT_UNPRIVILEGED_PORT_MIN, DEFAULT_UNPRIVILEGED_PORT_MAX, CAOS_DBPORT), CAOS_DBPORT_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database Name -----------------------------------------------------------------------------------
#define CAOS_DBNAME_DEFAULT     "my_caos_aweseome_app"
#define CAOS_DBNAME_DEFAULT_ALT "my_caos_aweseome_app_alt"

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #undef CAOS_DBNAME

  #ifdef CAOS_DBNAME_ALT
    #define CAOS_DBNAME CAOS_DBNAME_ALT
  #else
    #define CAOS_DBNAME CAOS_DBNAME_DEFAULT_ALT
  #endif
#endif

#ifndef CAOS_DBNAME
  #define CAOS_DBNAME CAOS_DBNAME_DEFAULT_ALT
#endif

#define CAOS_DBNAME_ERRMSG "CAOS_DBNAME" APPEND_ERRMSG_NON_NULL
static_assert(is_non_null_string(CAOS_DBNAME), CAOS_DBNAME_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database poolsize min ---------------------------------------------------------------------------
#define CAOS_DBPOOLSIZEMIN_DEFAULT 10
#define CAOS_DBPOOLSIZEMIN_LIMIT_MIN 1

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBPOOLSIZEMIN_ALT
    #undef CAOS_DBPOOLSIZEMIN
    #define CAOS_DBPOOLSIZEMIN CAOS_DBPOOLSIZEMIN_ALT
  #endif
#endif

#ifndef CAOS_DBPOOLSIZEMIN
  #define CAOS_DBPOOLSIZEMIN CAOS_DBPOOLSIZEMIN_DEFAULT
#endif

#define CAOS_DBPOOLSIZEMIN_ERRMSG "CAOS_DBPOOLSIZEMIN" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBPOOLSIZEMIN_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBPOOLSIZEMIN>(CAOS_DBPOOLSIZEMIN_LIMIT_MIN), CAOS_DBPOOLSIZEMIN_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database poolsize max ---------------------------------------------------------------------------
#define CAOS_DBPOOLSIZEMAX_DEFAULT 20
#define CAOS_DBPOOLSIZEMAX_LIMIT_MIN 1

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBPOOLSIZEMAX_ALT
    #undef CAOS_DBPOOLSIZEMAX
    #define CAOS_DBPOOLSIZEMAX CAOS_DBPOOLSIZEMAX_ALT
  #endif
#endif

#ifndef CAOS_DBPOOLSIZEMAX
  #define CAOS_DBPOOLSIZEMAX CAOS_DBPOOLSIZEMAX_DEFAULT
#endif

#define CAOS_DBPOOLSIZEMAX_ERRMSG "CAOS_DBPOOLSIZEMAX" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBPOOLSIZEMAX_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBPOOLSIZEMAX>(CAOS_DBPOOLSIZEMAX_LIMIT_MIN), CAOS_DBPOOLSIZEMAX_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Check if poolsize min < poolsize max ------------------------------------------------------------
#if CAOS_DBPOOLSIZEMIN > CAOS_DBPOOLSIZEMAX
  #error "CAOS_DBPOOLSIZEMIN > CAOS_DBPOOLSIZEMAX"
#endif
//--------------------------------------------------------------------------------------------------



// Database poolwait -------------------------------------------------------------------------------
#define CAOS_DBPOOLWAIT_DEFAULT 100
#define CAOS_DBPOOLWAIT_LIMIT_MIN 1

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBPOOLWAIT_ALT
    #undef CAOS_DBPOOLWAIT
    #define CAOS_DBPOOLWAIT CAOS_DBPOOLWAIT_ALT
  #endif
#endif

#ifndef CAOS_DBPOOLWAIT
  #define CAOS_DBPOOLWAIT CAOS_DBPOOLWAIT_DEFAULT
#endif

#define CAOS_DBPOOLWAIT_ERRMSG "CAOS_DBPOOLWAIT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBPOOLWAIT_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBPOOLWAIT>(CAOS_DBPOOLWAIT_LIMIT_MIN), CAOS_DBPOOLWAIT_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database pool timeout ---------------------------------------------------------------------------
#define CAOS_DBPOOLTIMEOUT_DEFAULT 15000
#define CAOS_DBPOOLTIMEOUT_LIMIT_MIN 1

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBPOOLTIMEOUT_ALT
    #undef CAOS_DBPOOLTIMEOUT
    #define CAOS_DBPOOLTIMEOUT CAOS_DBPOOLTIMEOUT_ALT
  #endif
#endif

#ifndef CAOS_DBPOOLTIMEOUT
  #define CAOS_DBPOOLTIMEOUT CAOS_DBPOOLTIMEOUT_DEFAULT
#endif

#define CAOS_DBPOOLTIMEOUT_ERRMSG "CAOS_DBPOOLTIMEOUT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBPOOLTIMEOUT_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBPOOLTIMEOUT>(CAOS_DBPOOLTIMEOUT_LIMIT_MIN), CAOS_DBPOOLTIMEOUT_ERRMSG);
//--------------------------------------------------------------------------------------------------



#ifdef CAOS_USE_DB_POSTGRESQL

  // Database keepalives ---------------------------------------------------------------------------
  #define CAOS_DBKEEPALIVES_DEFAULT    5
  #define CAOS_DBKEEPALIVES_LIMIT_MIN  0

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_DBKEEPALIVES_ALT
      #undef CAOS_DBKEEPALIVES
      #define CAOS_DBKEEPALIVES CAOS_DBKEEPALIVES_ALT
    #endif
  #endif

  #ifndef CAOS_DBKEEPALIVES
    #define CAOS_DBKEEPALIVES CAOS_DBKEEPALIVES_DEFAULT
  #endif

  #define CAOS_DBKEEPALIVES_ERRMSG "CAOS_DBKEEPALIVES" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBKEEPALIVES_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_DBKEEPALIVES>(CAOS_DBKEEPALIVES_LIMIT_MIN), CAOS_DBKEEPALIVES_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Database keepalives idle ----------------------------------------------------------------------
  #define CAOS_DBKEEPALIVES_IDLE_DEFAULT    50
  #define CAOS_DBKEEPALIVES_IDLE_LIMIT_MIN  0

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_DBKEEPALIVES_IDLE_ALT
      #undef CAOS_DBKEEPALIVES_IDLE
      #define CAOS_DBKEEPALIVES_IDLE CAOS_DBKEEPALIVES_IDLE_ALT
    #endif
  #endif

  #ifndef CAOS_DBKEEPALIVES_IDLE
    #define CAOS_DBKEEPALIVES_IDLE CAOS_DBKEEPALIVES_IDLE_DEFAULT
  #endif

  #define CAOS_DBKEEPALIVES_IDLE_ERRMSG "CAOS_DBKEEPALIVES_IDLE" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBKEEPALIVES_IDLE_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_DBKEEPALIVES_IDLE>(CAOS_DBKEEPALIVES_IDLE_LIMIT_MIN), CAOS_DBKEEPALIVES_IDLE_ERRMSG);
  //------------------------------------------------------------------------------------------------




  // Database keepalives interval ------------------------------------------------------------------
  #define CAOS_DBKEEPALIVES_INTERVAL_DEFAULT    30
  #define CAOS_DBKEEPALIVES_INTERVAL_LIMIT_MIN  0

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_DBKEEPALIVES_INTERVAL_ALT
      #undef CAOS_DBKEEPALIVES_INTERVAL
      #define CAOS_DBKEEPALIVES_INTERVAL CAOS_DBKEEPALIVES_INTERVAL_ALT
    #endif
  #endif

  #ifndef CAOS_DBKEEPALIVES_INTERVAL
    #define CAOS_DBKEEPALIVES_INTERVAL CAOS_DBKEEPALIVES_INTERVAL_DEFAULT
  #endif

  #define CAOS_DBKEEPALIVES_INTERVAL_ERRMSG "CAOS_DBKEEPALIVES_INTERVAL" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBKEEPALIVES_INTERVAL_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_DBKEEPALIVES_INTERVAL>(CAOS_DBKEEPALIVES_INTERVAL_LIMIT_MIN), CAOS_DBKEEPALIVES_INTERVAL_ERRMSG);
  //------------------------------------------------------------------------------------------------



  // Database keepalives count ---------------------------------------------------------------------
  #define CAOS_DBKEEPALIVES_COUNT_DEFAULT    10
  #define CAOS_DBKEEPALIVES_COUNT_LIMIT_MIN  0

  #ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
    #ifdef CAOS_DBKEEPALIVES_COUNT_ALT
      #undef CAOS_DBKEEPALIVES_COUNT
      #define CAOS_DBKEEPALIVES_COUNT CAOS_DBKEEPALIVES_COUNT_ALT
    #endif
  #endif

  #ifndef CAOS_DBKEEPALIVES_COUNT
    #define CAOS_DBKEEPALIVES_COUNT CAOS_DBKEEPALIVES_COUNT_DEFAULT
  #endif

  #define CAOS_DBKEEPALIVES_COUNT_ERRMSG "CAOS_DBKEEPALIVES_COUNT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBKEEPALIVES_COUNT_LIMIT_MIN)
  static_assert(is_number_non_null_and_at_least<CAOS_DBKEEPALIVES_COUNT>(CAOS_DBKEEPALIVES_COUNT_LIMIT_MIN), CAOS_DBKEEPALIVES_COUNT_ERRMSG);
  //------------------------------------------------------------------------------------------------

#endif // End Of CAOS_USE_DB_POSTGRESQL



// Database pool connection timeout ----------------------------------------------------------------
#define CAOS_DBCONNECT_TIMEOUT_DEFAULT    30
#define CAOS_DBCONNECT_TIMEOUT_LIMIT_MIN  1

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBCONNECT_TIMEOUT_ALT
    #undef CAOS_DBCONNECT_TIMEOUT
    #define CAOS_DBCONNECT_TIMEOUT CAOS_DBCONNECT_TIMEOUT_ALT
  #endif
#endif

#ifndef CAOS_DBCONNECT_TIMEOUT
  #define CAOS_DBCONNECT_TIMEOUT CAOS_DBCONNECT_TIMEOUT_DEFAULT
#endif

#define CAOS_DBCONNECT_TIMEOUT_ERRMSG "CAOS_DBCONNECT_TIMEOUT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBCONNECT_TIMEOUT_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBCONNECT_TIMEOUT>(CAOS_DBCONNECT_TIMEOUT_LIMIT_MIN), CAOS_DBCONNECT_TIMEOUT_ERRMSG);
//------------------------------------------------------------------------------------------------



// Database pool max wait --------------------------------------------------------------------------
#define CAOS_DBMAXWAIT_DEFAULT    5000
#define CAOS_DBMAXWAIT_LIMIT_MIN  0

#ifdef CAOS_ENV_ALT                                                                               // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBMAXWAIT_ALT
    #undef CAOS_DBMAXWAIT
    #define CAOS_DBMAXWAIT CAOS_DBMAXWAIT_ALT
  #endif
#endif

#ifndef CAOS_DBMAXWAIT
  #define CAOS_DBMAXWAIT CAOS_DBMAXWAIT_DEFAULT
#endif

#define CAOS_DBMAXWAIT_ERRMSG "CAOS_DBMAXWAIT" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBMAXWAIT_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBMAXWAIT>(CAOS_DBMAXWAIT_LIMIT_MIN), CAOS_DBMAXWAIT_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database pool connection timeout ----------------------------------------------------------------
#define CAOS_DBHEALTHCHECKINTERVAL_DEFAULT    30000
#define CAOS_DBHEALTHCHECKINTERVAL_LIMIT_MIN  0

#ifdef CAOS_ENV_ALT                                                                                 // CAOS_ENV="test" or CAOS_ENV="debug"
  #ifdef CAOS_DBHEALTHCHECKINTERVAL_ALT
    #undef CAOS_DBHEALTHCHECKINTERVAL
    #define CAOS_DBHEALTHCHECKINTERVAL CAOS_DBHEALTHCHECKINTERVAL_ALT
  #endif
#endif

#ifndef CAOS_DBHEALTHCHECKINTERVAL
  #define CAOS_DBHEALTHCHECKINTERVAL CAOS_DBHEALTHCHECKINTERVAL_DEFAULT
#endif

#define CAOS_DBHEALTHCHECKINTERVAL_ERRMSG "CAOS_DBHEALTHCHECKINTERVAL" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_DBHEALTHCHECKINTERVAL_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_DBHEALTHCHECKINTERVAL>(CAOS_DBHEALTHCHECKINTERVAL_LIMIT_MIN), CAOS_DBHEALTHCHECKINTERVAL_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database log threshold connection limit exceeded ++++++++++++++++++++++++++++++++++++++++++++++++
#define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_DEFAULT    50
#define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_LIMIT_MIN  0

#ifndef CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED
  #define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_DEFAULT
#endif

#define CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ERRMSG "CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED" APPEND_ERRMSG_AT_LEAST TOSTRING(CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_LIMIT_MIN)
static_assert(is_number_non_null_and_at_least<CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED>(CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_LIMIT_MIN), CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_ERRMSG);
//--------------------------------------------------------------------------------------------------








// Database validate connection before acquire +++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_DFAULT 1

#ifndef CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE
  #define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_DFAULT
#endif

#define CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ERRMSG "CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE" APPEND_ERRMSG_NON_BOOL
static_assert(is_bool<CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE>(), CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_ERRMSG);
//--------------------------------------------------------------------------------------------------



// Database validate using transaction -------------------------------------------------------------
#define CAOS_VALIDATE_USING_TRANSACTION_DFAULT 0

#ifndef CAOS_VALIDATE_USING_TRANSACTION
  #define CAOS_VALIDATE_USING_TRANSACTION CAOS_VALIDATE_USING_TRANSACTION_DFAULT
#endif

#define CAOS_VALIDATE_USING_TRANSACTION_ERRMSG "CAOS_VALIDATE_USING_TRANSACTION" APPEND_ERRMSG_NON_BOOL
static_assert(is_bool<CAOS_VALIDATE_USING_TRANSACTION>(), "CAOS_VALIDATE_USING_TRANSACTION must be a defined boolean");
//--------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
// End Of Database
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
