/**
 * CAOS PHP Extension
 *
 * This extension provides PHP bindings for the CAOS library with integrated
 * authentication system using execution hooks and call context.
 *
 * Authentication Flow:
 * 1. Hook intercepts all internal function calls (zend_execute_internal)
 * 2. Functions starting with "IQuery_" require context validation
 * 3. First parameter must be a call_context array
 * 4. Token is validated by caosFilter::Auth::Token class (if present in context)
 * 5. If validation fails, function returns error object without executing
 * 6. If validation succeeds, function executes normally
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

#include "extension_config.h"
#include "call_context.hpp"
#include "caosdba.hpp"
#include "error.hpp"

// =================================================================================================
// GLOBAL STATE AND INITIALIZATION
// =================================================================================================

/**
 * Type definition for the original zend_execute_internal function pointer
 * This is used to chain our hook with the original PHP execution handler
 */
typedef void (*zend_execute_internal_t)(zend_execute_data *execute_data, zval *return_value);
static zend_execute_internal_t original_execute_internal = NULL;

// =================================================================================================
// AUTHENTICATION HOOK
// =================================================================================================

/**
 * Authentication hook for internal PHP functions
 *
 * This hook intercepts ALL internal function calls (functions written in C, not PHP).
 * For functions starting with "IQuery_", it performs context-based authentication:
 *
 * 1. Extracts the first parameter as call_context array
 * 2. Creates CallContext object from array
 * 3. Validates context using registered validators
 * 4. If validation fails, returns error object immediately
 * 5. If validation succeeds, calls the original function
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

            // Extract first parameter as call_context array
            zval *context_param = ZEND_CALL_ARG(execute_data, 1);

            if (context_param && Z_TYPE_P(context_param) == IS_ARRAY)
            {
#ifdef CAOS_PHP_DEBUG
                php_printf("CONTEXT RECEIVED as array\n");
#endif
                try
                {
                    // Create CallContext from PHP array
                    CallContext ctx = CallContext::from_php_array(context_param, ZSTR_VAL(function_name));

                    // Validate
                    ctx.apply_auth_filters(ZSTR_VAL(function_name));

#ifdef CAOS_PHP_DEBUG
                    php_printf("CONTEXT VALIDATION: SUCCESS\n");
#endif
                }
                catch (const CallContext::AuthError& e)
                {
                    // Authentication failed
#ifdef CAOS_PHP_DEBUG
                    php_printf("AUTH FAILED: %s\n", e.what());
#endif
                    create_auth_error(return_value, e.what());
                    return;
                }
                catch (const CallContext::ValidationError& e)
                {
                    // Context validation failed
#ifdef CAOS_PHP_DEBUG
                    php_printf("VALIDATION FAILED: %s\n", e.what());
#endif
                    create_error_object(return_value, e.error_type().c_str(), e.what());
                    return;
                }
                catch (const std::exception& e)
                {
                    // System error during validation
#ifdef CAOS_PHP_DEBUG
                    php_printf("SYSTEM ERROR: %s\n", e.what());
#endif
                    create_system_error(return_value, e.what());
                    return;
                }
            }
            else
            {
                // No context parameter found or wrong type
#ifdef CAOS_PHP_DEBUG
                php_printf("ERROR: First parameter must be a call_context array\n");
#endif
                create_parameter_error(return_value, "First parameter must be a call_context array");
                return;
            }
        }
    }

    // Context validated successfully OR not a CAOS function - proceed with normal execution
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
#ifdef QUERY_EXISTS_IQuery_Template_echoString
PHP_FUNCTION(IQuery_Template_echoString);
#endif

#ifdef QUERY_EXISTS_IQuery_Template_echoString_custom
PHP_FUNCTION(IQuery_Template_echoString_custom);
#endif

PHP_FUNCTION(info);

// =================================================================================================
// ARGUMENT INFO
// =================================================================================================

/**
 * Argument information for IQuery_Template_echoString()
 *
 * Parameters:
 * 1. call_context (array) - Call context array (with optional token)
 * 2. str (string)         - String to echo
 */
#ifdef QUERY_EXISTS_IQuery_Template_echoString
ZEND_BEGIN_ARG_INFO(arginfolibIQuery_Template_echoString, 0)
ZEND_ARG_TYPE_INFO(0, call_context, IS_ARRAY, 0)
ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
#endif

/**
 * Argument information for IQuery_Template_echoString_custom()
 *
 * Parameters:
 * 1. call_context (array) - Call context array (with optional token)
 * 2. str (string)         - String to echo
 */
#ifdef QUERY_EXISTS_IQuery_Template_echoString_custom
ZEND_BEGIN_ARG_INFO(arginfolibIQuery_Template_echoString_custom, 0)
ZEND_ARG_TYPE_INFO(0, call_context, IS_ARRAY, 0)
ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
#endif

/**
 * Argument information for info()
 * No parameters
 */
ZEND_BEGIN_ARG_INFO(arginfo_info, 0)
ZEND_END_ARG_INFO()


// =================================================================================================
// FUNCTION REGISTRATION
// =================================================================================================

/**
 * List of functions exposed to PHP
 */
static const zend_function_entry PHP_EXT_FUNCTIONS[] =
{
#ifdef QUERY_EXISTS_IQuery_Template_echoString
    PHP_FE(IQuery_Template_echoString, arginfolibIQuery_Template_echoString)
#endif

#ifdef QUERY_EXISTS_IQuery_Template_echoString_custom
    PHP_FE(IQuery_Template_echoString_custom, arginfolibIQuery_Template_echoString_custom)
#endif
    PHP_FE(info, arginfo_info)
    PHP_FE_END
};

// =================================================================================================
// MODULE LIFECYCLE
// =================================================================================================

/**
 * Module initialization (called when extension is loaded)
 *
 * Installs the authentication hook by replacing zend_execute_internal
 * with our custom handler that performs context validation
 * Registers default validators for call context
 *
 * @return SUCCESS on successful initialization
 */
PHP_EXT_MINIT_FUNCTION
{
    (void)type;
    (void)module_number;

    // Save original handler and install our hook
#ifdef CAOS_PHP_DEBUG
    php_printf("CAOS[%s]: Installing INTERNAL execution hook\n", PHP_EXT_NAME_STR);
#endif
    original_execute_internal = zend_execute_internal;
    zend_execute_internal = caos_execute_internal;

    // Register module constants
    REGISTER_STRING_CONSTANT(PHP_EXT_NAME_STR "_BUILD", get_build_count_string(), CONST_PERSISTENT | CONST_CS);

#ifdef CAOS_PHP_DEBUG
    php_printf("CAOS[%s]: Internal hook installed successfully\n", PHP_EXT_NAME_STR);
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
PHP_EXT_MSHUTDOWN_FUNCTION
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
PHP_EXT_MINFO_FUNCTION
{
    (void)zend_module;

    php_info_print_table_start();
    php_info_print_table_header(2, PHP_EXT_NAME_STR " PHP Extension", "enabled");
    php_info_print_table_row(2, "Build", get_build_count_string());
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
    PHP_EXT_NAME_STR,
    PHP_EXT_FUNCTIONS,
    PHP_EXT_MINIT_CALL,
    PHP_EXT_MSHUTDOWN_CALL,
    NULL,
    NULL,
    PHP_EXT_MINFO_CALL,
    get_build_count_string(),
    STANDARD_MODULE_PROPERTIES
};

extern "C"
{
    ZEND_EXT_GET_MODULE
}

// =================================================================================================
// FUNCTION IMPLEMENTATIONS
// =================================================================================================

/**
 * IQuery_Template_echoString() - Echo string through CAOS repository
 *
 * This function demonstrates:
 * 1. Context-based authentication (via hook)
 * 2. Parameter validation
 * 3. Repository interaction
 * 4. Structured error handling
 *
 * The authentication hook intercepts this call and validates the context
 * BEFORE this function body executes. If context validation fails, this
 * function is never called.
 *
 * Usage: IQuery_Template_echoString(['token' => '...'], $string)
 *
 * @param array $call_context Call context array (validated by hook)
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
 * - VALIDATION: Context validation failure (set by hook)
 */
#ifdef QUERY_EXISTS_IQuery_Template_echoString
PHP_FUNCTION(IQuery_Template_echoString)
{
    (void)execute_data;

    zval *context_zval;
    char *str;
    size_t str_length;

    // Parse parameters: call_context (validated by hook) and string
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "as", &context_zval, &str, &str_length) == FAILURE)
    {
        // Parameter parsing failed - return structured error
        create_parameter_error(return_value, "Missing parameters (call_context array, string expected)");
        return;
    }

    // NOTE: CallContext has already been validated by the authentication hook
    // If we reach this point, context validation was successful

    try
    {
        // Get repository instance
        Cache* repo = Repository();
        if (!repo)
        {
            // Repository not available - system error
            create_system_error(return_value, "CAOS repository not available");
            return;
        }

        // Call repository method
        auto repo_result = repo->IQuery_Template_echoString(str);
        if (!repo_result.has_value())
        {
            // Repository returned no value - return success with null data
            create_success_object(return_value, nullptr);
            return;
        }

        // Build final result string
        std::string final_result = repo_result.value() + " through PHP!\n";

        // Create result string zval
        zval result_data;
        ZVAL_STRINGL(&result_data, final_result.c_str(), final_result.length());

        // Return success with data
        create_success_object(return_value, &result_data);

        // Clean up the temporary zval
        zval_ptr_dtor(&result_data);
    }
    catch (const std::exception& e)
    {
        // Exception during execution - return system error
        create_system_error(return_value, e.what());
    }
}
#endif

/**
 * IQuery_Template_echoString_custom() - Echo string through CAOS repository
 *
 * This function demonstrates:
 * 1. Context-based authentication (via hook)
 * 2. Parameter validation
 * 3. Repository interaction
 * 4. Structured error handling
 *
 * The authentication hook intercepts this call and validates the context
 * BEFORE this function body executes. If context validation fails, this
 * function is never called.
 *
 * Usage: IQuery_Template_echoString_custom(['token' => '...'], $string)
 *
 * @param array $call_context Call context array (validated by hook)
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
 * - VALIDATION: Context validation failure (set by hook)
 */
#ifdef QUERY_EXISTS_IQuery_Template_echoString_custom
PHP_FUNCTION(IQuery_Template_echoString_custom)
{
    (void)execute_data;

    zval *context_zval;
    char *str;
    size_t str_length;

    // Parse parameters: call_context (validated by hook) and string
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "as", &context_zval, &str, &str_length) == FAILURE)
    {
        // Parameter parsing failed - return structured error
        create_parameter_error(return_value, "Missing parameters (call_context array, string expected)");
        return;
    }

    // NOTE: CallContext has already been validated by the authentication hook
    // If we reach this point, context validation was successful

    try
    {
        // Get repository instance
        Cache* repo = Repository();
        if (!repo)
        {
            // Repository not available - system error
            create_system_error(return_value, "CAOS repository not available");
            return;
        }

        // Call repository method
        auto repo_result = repo->IQuery_Template_echoString_custom(str);
        if (!repo_result.has_value())
        {
            // Repository returned no value - return success with null data
            create_success_object(return_value, nullptr);
            return;
        }

        // Build final result string
        std::string final_result = repo_result.value() + " through PHP!\n";

        // Create result string zval
        zval result_data;
        ZVAL_STRINGL(&result_data, final_result.c_str(), final_result.length());

        // Return success with data
        create_success_object(return_value, &result_data);

        // Clean up the temporary zval
        zval_ptr_dtor(&result_data);
    }
    catch (const std::exception& e)
    {
        // Exception during execution - return system error
        create_system_error(return_value, e.what());
    }
}
#endif

/**
 * Get build information about the PHP extension
 *
 * @return array Build information including module name, version, build number, etc.
 */
PHP_FUNCTION(info)
{
    (void)execute_data;

    array_init(return_value);

    // Module name
    add_assoc_string(return_value, "module", PHP_EXT_NAME_STR);

    // Build
    add_assoc_string(return_value, "build", get_build_info_string());
}
