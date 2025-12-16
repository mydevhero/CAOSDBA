#pragma once

#include <php.h>

/**
 * Create standardized PHP error object into return_value
 *
 * @param return_value Where to store the error object
 * @param error_type Error category (PARAMETER, SYSTEM, AUTH, etc.)
 * @param error_message Human-readable error description
 * @param additional_data Optional zval with additional error data
 */
static void create_error_object(zval* return_value,
                                const char* error_type,
                                const char* error_message,
                                zval* additional_data = nullptr)
{
  object_init(return_value);

  // success: false
  add_property_bool(return_value, "success", false);

  // error_type
  add_property_string(return_value, "error_type", error_type);

  // error_message
  add_property_string(return_value, "error_message", error_message);

  // data
  if (additional_data)
  {
    add_property_zval(return_value, "data", additional_data);
  }
  else
  {
    add_property_null(return_value, "data");
  }
}

/**
 * Create standardized PHP success object into return_value
 *
 * @param return_value Where to store the success object
 * @param data Result data (zval*)
 */
static void create_success_object(zval* return_value, zval* data)
{
  object_init(return_value);

  // success: true
  add_property_bool(return_value, "success", true);

  // data
  if (data)
  {
    add_property_zval(return_value, "data", data);
  }
  else
  {
    add_property_null(return_value, "data");
  }
}

/**
 * Helper for parameter parsing errors
 */
static void create_parameter_error(zval* return_value, const char* message)
{
  create_error_object(return_value, "PARAMETER", message);
}

/**
 * Helper for system errors
 */
static void create_system_error(zval* return_value, const char* message)
{
  create_error_object(return_value, "SYSTEM", message);
}

/**
 * Helper for authentication errors
 */
static void create_auth_error(zval* return_value, const char* message)
{
  create_error_object(return_value, "AUTH", message);
}
