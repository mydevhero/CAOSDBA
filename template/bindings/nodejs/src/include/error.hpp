#pragma once

#include <napi.h>
#include <string>

// =================================================================================================
// ERROR HANDLING UTILITIES FOR N-API
// =================================================================================================

/**
 * Create standardized error object for Node.js
 */
static Napi::Object CreateErrorObject(Napi::Env env,
                                      const std::string& error_type,
                                      const std::string& error_message)
{
  Napi::Object dict = Napi::Object::New(env);

  // success: false
  dict.Set("success", Napi::Boolean::New(env, false));

  // error_type
  dict.Set("error_type", Napi::String::New(env, error_type));

  // error_message
  dict.Set("error_message", Napi::String::New(env, error_message));

  // data is null by default
  dict.Set("data", env.Null());

  return dict;
}

/**
 * Create standardized error object with additional data
 */
static Napi::Object CreateErrorObject(Napi::Env env,
                                      const std::string& error_type,
                                      const std::string& error_message,
                                      Napi::Value additional_data)
{
  Napi::Object dict = Napi::Object::New(env);

  // success: false
  dict.Set("success", Napi::Boolean::New(env, false));

  // error_type
  dict.Set("error_type", Napi::String::New(env, error_type));

  // error_message
  dict.Set("error_message", Napi::String::New(env, error_message));

  // data
  dict.Set("data", additional_data);

  return dict;
}

/**
 * Create standardized success object for Node.js
 */
static Napi::Object CreateSuccessObject(Napi::Env env, Napi::Value data)
{
  Napi::Object dict = Napi::Object::New(env);

  // success: true
  dict.Set("success", Napi::Boolean::New(env, true));

  // data
  dict.Set("data", data);

  return dict;
}

/**
 * Helper for parameter parsing errors
 */
static Napi::Object CreateParameterError(Napi::Env env, const std::string& message)
{
  return CreateErrorObject(env, "PARAMETER", message);
}

/**
 * Helper for system errors
 */
static Napi::Object CreateSystemError(Napi::Env env, const std::string& message)
{
  return CreateErrorObject(env, "SYSTEM", message);
}
