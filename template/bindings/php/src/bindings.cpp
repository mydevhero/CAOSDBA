/**
 * CAOS PHP Extension
 *
 * This extension provides PHP bindings for the CAOS library with integrated
 * authentication system using execution hooks.
 *
 * Authentication Flow:
 * 1. Hook intercepts all internal function calls (zend_execute_internal)
 * 2. Functions starting with "IQuery_" require token validation
 * 3. Token is validated by caosFilter::Auth::Token class
 * 4. If validation fails, function returns error object without executing
 * 5. If validation succeeds, function executes normally
 */

// #define CAOS_PHP_DEBUG
// #define CAOS_PHP_REPOSITORY_EARLY_START


extern "C" {
#pragma GCC diagnostic ignored "-Wc++20-extensions"
#include <php.h>
#include <zend_API.h>
#include <zend_modules.h>
#include <ext/standard/info.h>
}

#include <libcaos.hpp>
#include <build_info.h>
#include <extension_config.h>

// =================================================================================================
// GLOBAL STATE AND INITIALIZATION
// =================================================================================================

/**
 * Type definition for the original zend_execute_internal function pointer
 * This is used to chain our hook with the original PHP execution handler
 */
typedef void (*zend_execute_internal_t)(zend_execute_data *execute_data, zval *return_value);
static zend_execute_internal_t original_execute_internal = NULL;

/**
 * Get or initialize the global CAOS instance
 *
 * @param check If true, only check if instance exists without initializing
 * @return Reference to the unique_ptr holding the CAOS instance
 *
 * The CAOS instance is initialized lazily on first access with:
 * - Command line arguments from environment (CAOS_CONFIG if set)
 * - Repository initialization flag
 */
std::unique_ptr<Caos>& libcaos(bool check=false)
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (check)
  {
    return caos;
  }

  if (!caos)
  {
    // Build fake command line arguments
    std::vector<std::string> args = {"caos_php"};
    std::vector<char*> argv;
    for (auto& arg : args)
    {
      argv.push_back(&arg[0]);
    }
    int argc = argv.size();

    // Initialize CAOS with repository
    caos = std::make_unique<Caos>(argc, argv.data(), (initFlags::Repository|initFlags::PHP_EXT));
  }

  return caos;
}

/**
 * Get the CAOS repository/cache instance
 *
 * @return Pointer to Cache instance, or nullptr if not available
 */
Cache* fromRepository()
{
  auto& caos = libcaos();

  return caos ? caos->repository.get() : nullptr;
}

// =================================================================================================
// AUTHENTICATION HOOK
// =================================================================================================

/**
 * Authentication hook for internal PHP functions
 *
 * This hook intercepts ALL internal function calls (functions written in C, not PHP).
 * For functions starting with "IQuery_", it performs token-based authentication:
 *
 * 1. Extracts the first parameter as authentication token
 * 2. Validates token using caosFilter::Auth::Token
 * 3. If validation fails, returns error object immediately
 * 4. If validation succeeds, calls the original function
 *
 * @param execute_data Execution context containing function info and parameters
 * @param return_value Pointer to where the return value should be stored
 */
static void caos_execute_internal(zend_execute_data *execute_data, zval *return_value)
{
  // Check if we have valid function information
  if (execute_data && execute_data->func && execute_data->func->common.function_name)
  {
    zend_string *function_name = execute_data->func->common.function_name;

#ifdef CAOS_PHP_DEBUG
    php_printf("INTERNAL HOOK: %s (NumArgs: %d)\n", ZSTR_VAL(function_name), ZEND_NUM_ARGS());
#endif

    // Check if this is a CAOS query function (IQuery_* pattern)
    if (zend_string_starts_with_literal(function_name, "IQuery_"))
    {
#ifdef CAOS_PHP_DEBUG
      php_printf("CAOS FUNCTION INTERCEPTED: %s\n", ZSTR_VAL(function_name));
#endif

      // Extract first parameter as authentication token
      zval *token_param = ZEND_CALL_ARG(execute_data, 1);

      if (token_param && Z_TYPE_P(token_param) == IS_STRING)
      {
#ifdef CAOS_PHP_DEBUG
        php_printf("TOKEN RECEIVED: %s\n", Z_STRVAL_P(token_param));
#endif
        try
        {
          // Validate token using CAOS authentication system
          // The Token class internally decides if this specific function requires authentication
          bool token_valid = caosFilter::Auth::Token(ZSTR_VAL(function_name))
                             .validate(Z_STRVAL_P(token_param), Z_STRLEN_P(token_param));

#ifdef CAOS_PHP_DEBUG
          php_printf("TOKEN VALIDATION: %s\n", token_valid ? "SUCCESS" : "FAILED");
#endif

          if (!token_valid)
          {
            // Authentication failed - return error object without executing function
#ifdef CAOS_PHP_DEBUG
            php_printf("AUTH FAILED - Returning error object\n");
#endif
            object_init(return_value);
            add_property_bool(return_value, "success", false);
            add_property_string(return_value, "error_type", "AUTH");
            add_property_string(return_value, "error_message", "Token validation failed");
            add_property_null(return_value, "data");
            return; // Exit without calling original_execute_internal
          }

#ifdef CAOS_PHP_DEBUG
          php_printf("AUTH SUCCESS - Executing function\n");
#endif
        }
        catch (const std::exception& e)
        {
          // Authentication system error (e.g., connection failure, malformed token)
#ifdef CAOS_PHP_DEBUG
          php_printf("AUTH EXCEPTION: %s\n", e.what());
#endif

          object_init(return_value);
          add_property_bool(return_value, "success", false);
          add_property_string(return_value, "error_type", "AUTH_SYSTEM_ERROR");
          add_property_string(return_value, "error_message", e.what());
          add_property_null(return_value, "data");
          return;
        }
      }
      else
      {
        // No token parameter found or wrong type
        // This might indicate a programming error or function that doesn't require auth
#ifdef CAOS_PHP_DEBUG
        php_printf("INFO: No token parameter found or wrong type\n");
#endif
      }
    }
  }

  // Token validated successfully OR not a CAOS function - proceed with normal execution
  if (original_execute_internal)
  {
    // Call the original handler (might be from another extension)
    original_execute_internal(execute_data, return_value);
  }
  else
  {
    // No previous handler - call default PHP internal executor
    execute_internal(execute_data, return_value);
  }
}

// =================================================================================================
// FUNCTION DECLARATIONS
// =================================================================================================
PHP_FUNCTION(IQuery_Demo_Test_echoString);

// =================================================================================================
// ARGUMENT INFO
// =================================================================================================

/**
 * Argument information for IQuery_Demo_Test_echoString()
 *
 * Parameters:
 * 1. token (string)  - Authentication token
 * 2. str (string)    - String to echo
 */
ZEND_BEGIN_ARG_INFO(arginfolibIQuery_Demo_Test_echoString, 0)
ZEND_ARG_INFO(0, token)  // Token as first parameter
ZEND_ARG_INFO(0, str)    // Original parameter as second
ZEND_END_ARG_INFO()

// =================================================================================================
// FUNCTION REGISTRATION
// =================================================================================================

/**
 * List of functions exposed to PHP
 */
static const zend_function_entry PHP_EXT_FUNCTIONS[] =
{
  PHP_FE(IQuery_Demo_Test_echoString, arginfolibIQuery_Demo_Test_echoString)
  PHP_FE_END
};

// =================================================================================================
// MODULE LIFECYCLE
// =================================================================================================

/**
 * Module initialization (called when extension is loaded)
 *
 * Installs the authentication hook by replacing zend_execute_internal
 * with our custom handler that performs token validation
 *
 * @return SUCCESS on successful initialization
 */
PHP_EXT_MINIT_FUNCTION // PHP_MINIT_FUNCTION(...)
{
  (void)type;
  (void)module_number;

#ifdef CAOS_PHP_REPOSITORY_EARLY_START
  auto& caos = libcaos();
#endif

  // Save original handler and install our hook
#ifdef CAOS_PHP_DEBUG
  php_printf("CAOS[${CAOS_PROJECT_NAME}]: Installing INTERNAL execution hook\n");
#endif
  original_execute_internal = zend_execute_internal;
  zend_execute_internal = caos_execute_internal;
#ifdef CAOS_PHP_DEBUG
  php_printf("CAOS[${CAOS_PROJECT_NAME}]: Internal hook installed successfully\n");
#endif

  return SUCCESS;
}

/**
 * Module shutdown (called when extension is unloaded)
 *
 * Restores the original execution handler and cleans up CAOS instance
 *
 * @return SUCCESS on successful cleanup, FAILURE on error
 */
PHP_EXT_MSHUTDOWN_FUNCTION // PHP_MSHUTDOWN_FUNCTION(...)
{
  (void)type;
  (void)module_number;

  // Restore original execution handler
  if (original_execute_internal)
  {
    zend_execute_internal = original_execute_internal;
  }

  try
  {
    // Clean up CAOS instance if it exists
    auto& caos = libcaos(true);
    if (caos != nullptr)
    {
      caos.reset();
    }

    return SUCCESS;
  }
  catch (const std::exception& e)
  {
    php_error_docref(NULL, E_ERROR, "CAOS shutdown failed: %s", e.what());
    return FAILURE;
  }

  return SUCCESS;
}

/**
 * phpinfo() section
 *
 * Displays extension information in phpinfo() output
 */
PHP_EXT_MINFO_FUNCTION // PHP_MINFO_FUNCTION(...)
{
  (void)zend_module;

  php_info_print_table_start();
  php_info_print_table_header(2, PHP_EXT_NAME_STR " PHP Extension", "enabled");
  php_info_print_table_row(2, "Build Date", CAOS_BUILD_COUNT);
  php_info_print_table_end();
}

// =================================================================================================
// MODULE DEFINITION
// =================================================================================================

/**
 * Module entry structure
 * Defines the extension metadata and lifecycle hooks
 */
zend_module_entry PHP_EXT_MODULE_ENTRY =
{
  STANDARD_MODULE_HEADER,
  PHP_EXT_NAME_STR,           // Stringa
  PHP_EXT_FUNCTIONS,
  PHP_EXT_MINIT_CALL,         // Token
  PHP_EXT_MSHUTDOWN_CALL,     // Token
  NULL,
  NULL,
  PHP_EXT_MINFO_CALL,         // Token
  get_build_count_string(),
  STANDARD_MODULE_PROPERTIES
};

extern "C"
{
  ZEND_EXT_GET_MODULE // ZEND_GET_MODULE(...)
}

// =================================================================================================
// FUNCTION IMPLEMENTATIONS
// =================================================================================================

/**
 * IQuery_Demo_Test_echoString() - Echo string through CAOS repository
 *
 * This function demonstrates:
 * 1. Token-based authentication (via hook)
 * 2. Parameter validation
 * 3. Repository interaction
 * 4. Structured error handling
 *
 * The authentication hook intercepts this call and validates the token
 * BEFORE this function body executes. If token validation fails, this
 * function is never called.
 *
 * Usage: IQuery_Demo_Test_echoString($token, $string)
 *
 * @param string $token Authentication token (validated by hook)
 * @param string $str   String to echo through repository
 *
 * @return object Standardized result object:
 *   - success (bool): Operation success status
 *   - data (mixed): Result data on success, null on failure
 *   - error_type (string): Error category (only on failure)
 *   - error_message (string): Human-readable error (only on failure)
 *
 * Error types:
 * - PARAMETER: Invalid or missing parameters
 * - SYSTEM: Repository or system-level errors
 * - AUTH: Authentication failure (set by hook, not by this function)
 * - AUTH_SYSTEM_ERROR: Authentication system error (set by hook)
 */
PHP_FUNCTION(IQuery_Demo_Test_echoString)
{
  (void)execute_data;

  char *token;
  size_t token_length;
  char *str;
  size_t str_length;

  // Parse parameters: token (authenticated by hook) and string
  if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &token, &token_length, &str, &str_length) == FAILURE)
  {
    // Parameter parsing failed - return structured error
    zval result_obj;
    object_init(&result_obj);
    add_property_bool(&result_obj, "success", false);
    add_property_string(&result_obj, "error_type", "PARAMETER");
    add_property_string(&result_obj, "error_message", "Missing parameters (token, string expected)");
    add_property_null(&result_obj, "data");
    RETURN_OBJ(Z_OBJ(result_obj));
  }

  // NOTE: Token has already been validated by the authentication hook
  // If we reach this point, authentication was successful

  try
  {
    // Get repository instance
    Cache* repo = fromRepository();
    if (!repo)
    {
      // Repository not available - system error
      zval result_obj;
      object_init(&result_obj);
      add_property_bool(&result_obj, "success", false);
      add_property_string(&result_obj, "error_type", "SYSTEM");
      add_property_string(&result_obj, "error_message", "CAOS repository not available");
      add_property_null(&result_obj, "data");
      RETURN_OBJ(Z_OBJ(result_obj));
    }

    // Call repository method
    auto repo_result = repo->IQuery_Demo_Test_echoString(str);
    if (!repo_result.has_value())
    {
      // Repository returned no value - return success with null data
      zval result_obj;
      object_init(&result_obj);
      add_property_bool(&result_obj, "success", true);
      add_property_null(&result_obj, "data");
      RETURN_OBJ(Z_OBJ(result_obj));
    }

    // Build final result string
    std::string final_result = repo_result.value() + " through PHP!\n";

    // Return success with data
    zval result_obj;
    object_init(&result_obj);
    add_property_bool(&result_obj, "success", true);
    add_property_string(&result_obj, "data", final_result.c_str());
    RETURN_OBJ(Z_OBJ(result_obj));
  }
  catch (const std::exception& e)
  {
    // Exception during execution - return system error
    zval result_obj;
    object_init(&result_obj);
    add_property_bool(&result_obj, "success", false);
    add_property_string(&result_obj, "error_type", "SYSTEM");
    add_property_string(&result_obj, "error_message", e.what());
    add_property_null(&result_obj, "data");
    RETURN_OBJ(Z_OBJ(result_obj));
  }
}
