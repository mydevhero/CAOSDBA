#include <memory>
#include <vector>
#include <string>

#include <napi.h>
#include "extension_config.h"
#include "call_context.hpp"
#include "error.hpp"
#include "caosdba.hpp"

// =================================================================================================
// LAZY INITIALIZATION HELPER (N-API version)
// =================================================================================================

class CaosLazyInitializer
{
  private:
    static bool initialized_;

    static void initialize_if_needed()
    {
      if (!initialized_)
      {
        libcaos();
        initialized_ = true;
      }
    }

  public:
    static void ensure_initialized()
    {
      initialize_if_needed();
    }

    static bool is_initialized()
    {
      return initialized_;
    }

    static void cleanup()
    {
      if (initialized_)
      {
        caos_shutdown();
        initialized_ = false;
      }
    }
};

bool CaosLazyInitializer::initialized_ = false;

// =================================================================================================
// QUERY IMPLEMENTATION - IQuery_Template_echoString (N-API version)
// =================================================================================================
#ifdef QUERY_EXISTS_IQuery_Template_echoString
Napi::Value IQuery_Template_echoString(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    return CreateParameterError(env, "Invalid parameters for IQuery_Template_echoString");
  }

  if (!info[0].IsObject())
  {
    return CreateParameterError(env, "First argument must be a call_context object_custom");
  }

  if (!info[1].IsString())
  {
    return CreateParameterError(env, "Second argument must be a string");
  }

  try
  {
    // Initialize CAOS only when needed
    CaosLazyInitializer::ensure_initialized();

    // 1. Create call_context from JavaScript object
    Napi::Object call_context_obj = info[0].As<Napi::Object>();
    CallContext ctx = CallContext::from_napi_object(call_context_obj, "IQuery_Template_echoString");

    // 2. Apply auth filters (delegates to caosFilter)
    ctx.apply_auth_filters("IQuery_Template_echoString");

    // 3. Get input string
    std::string input_str = info[1].As<Napi::String>().Utf8Value();

    // 4. Execute query
    Cache* repo = Repository();
    if (!repo)
    {
      throw std::runtime_error("Repository not available");
    }

    auto result = repo->IQuery_Template_echoString(input_str);

    // 5. Convert result to Napi::Value
    if (!result.has_value())
    {
      return CreateSuccessObject(env, env.Null());
    }

    Napi::String result_str = Napi::String::New(env, result.value());
    return CreateSuccessObject(env, result_str);
  }
  catch (const CallContext::AuthError& e)
  {
    return CreateErrorObject(env, "AUTH", e.what());
  }
  catch (const CallContext::ValidationError& e)
  {
    return CreateErrorObject(env, e.error_type(), e.what());
  }
  catch (const std::exception& e)
  {
    return CreateErrorObject(env, "SYSTEM",
                             std::string("System error in IQuery_Template_echoString: ") + e.what());
  }
}
#endif

// =================================================================================================
// QUERY IMPLEMENTATION - IQuery_Template_echoString_custom (N-API version)
// =================================================================================================
#ifdef QUERY_EXISTS_IQuery_Template_echoString_custom
Napi::Value IQuery_Template_echoString_custom(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    return CreateParameterError(env, "Invalid parameters for IQuery_Template_echoString_custom");
  }

  if (!info[0].IsObject())
  {
    return CreateParameterError(env, "First argument must be a call_context object_custom");
  }

  if (!info[1].IsString())
  {
    return CreateParameterError(env, "Second argument must be a string");
  }

  try
  {
    // Initialize CAOS only when needed
    CaosLazyInitializer::ensure_initialized();

    // 1. Create call_context from JavaScript object
    Napi::Object call_context_obj = info[0].As<Napi::Object>();
    CallContext ctx = CallContext::from_napi_object(call_context_obj, "IQuery_Template_echoString_custom");

    // 2. Apply auth filters (delegates to caosFilter)
    ctx.apply_auth_filters("IQuery_Template_echoString_custom");

    // 3. Get input string
    std::string input_str = info[1].As<Napi::String>().Utf8Value();

    // 4. Execute query
    Cache* repo = Repository();
    if (!repo)
    {
      throw std::runtime_error("Repository not available");
    }

    auto result = repo->IQuery_Template_echoString_custom(input_str);

    // 5. Convert result to Napi::Value
    if (!result.has_value())
    {
      return CreateSuccessObject(env, env.Null());
    }

    Napi::String result_str = Napi::String::New(env, result.value());
    return CreateSuccessObject(env, result_str);
  }
  catch (const CallContext::AuthError& e)
  {
    return CreateErrorObject(env, "AUTH", e.what());
  }
  catch (const CallContext::ValidationError& e)
  {
    return CreateErrorObject(env, e.error_type(), e.what());
  }
  catch (const std::exception& e)
  {
    return CreateErrorObject(env, "SYSTEM",
                             std::string("System error in IQuery_Template_echoString_custom: ") + e.what());
  }
}
#endif
// =================================================================================================
// HELPER FUNCTIONS (N-API version)
// =================================================================================================

Napi::Value GetBuildInfo(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  Napi::Object dict = Napi::Object::New(env);

  std::string module_info = std::string(MODULE_DESCRIPTION)
                            + " ["
                            + get_build_info_string()
                            + "]";

  dict.Set("module", Napi::String::New(env, module_info));
  dict.Set("build", Napi::Number::New(env, CAOS_BUILD_COUNT));
  dict.Set("caos_initialized", Napi::Boolean::New(env, CaosLazyInitializer::is_initialized()));

  // Add Node.js specific info
  dict.Set("node_version", Napi::String::New(env, std::to_string(NODE_VERSION)));
  dict.Set("napi_version", Napi::Number::New(env, NAPI_VERSION));

#ifdef CAOS_NODE_DEBUG
  dict.Set("debug", Napi::Boolean::New(env, true));
#else
  dict.Set("debug", Napi::Boolean::New(env, false));
#endif

  return dict;
}

// =================================================================================================
// MODULE INITIALIZATION (N-API version)
// =================================================================================================

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  // Register cleanup handler for CAOS library
  napi_add_env_cleanup_hook(env, [](void* data) {
    CaosLazyInitializer::cleanup();
  }, nullptr);

  // Export query functions
  #ifdef QUERY_EXISTS_IQuery_Template_echoString
  exports.Set("IQuery_Template_echoString",
              Napi::Function::New(env, IQuery_Template_echoString));
  #endif

  #ifdef QUERY_EXISTS_IQuery_Template_echoString_custom
  exports.Set("IQuery_Template_echoString_custom",
              Napi::Function::New(env, IQuery_Template_echoString_custom));
  #endif

  // Export helper functions
  exports.Set("getBuildInfo",
              Napi::Function::New(env, GetBuildInfo));

  // Add module-level constants
  exports.Set("__version__", Napi::String::New(env, "0.1.0"));
  exports.Set("__auth_system__", Napi::String::New(env, "caosFilter::Auth::Token"));
  exports.Set("__call_context_format__",
              Napi::String::New(env, "object with optional 'token' field"));

#ifdef CAOS_NODE_DEBUG
  exports.Set("__debug__", Napi::Number::New(env, 1));
#else
  exports.Set("__debug__", Napi::Number::New(env, 0));
#endif

  return exports;
}

NODE_API_MODULE(caos, Init)
