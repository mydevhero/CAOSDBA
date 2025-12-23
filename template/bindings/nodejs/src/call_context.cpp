#include "call_context.hpp"
#include <libcaos.hpp>
#include <stdexcept>  // <-- Utile per std::exception

// ============================================================================
// CallContext Implementation
// ============================================================================

CallContext CallContext::from_napi_object(const Napi::Object& obj, const std::string& query_name)
{
  if (!obj.IsObject())
  {
    throw ValidationError("call_context must be an object", "PARAMETER");
  }

  Data data;

  // Extract token from JavaScript object
  if (obj.Has("token"))
  {
    Napi::Value token_val = obj.Get("token");
    if (token_val.IsString())
    {
      std::string token_str = token_val.As<Napi::String>().Utf8Value();
      if (!token_str.empty())
      {
        data.token = token_str;
      }
    }
  }

  return CallContext(std::move(data));
}

void CallContext::apply_auth_filters(const std::string& query_name) const
{
  try
  {
    // Delegate ALL validation to caosFilter
    // caosFilter knows QUERY_AUTH_MAP and decides if token is required
    bool valid = caosFilter::Auth::Token(query_name.c_str())
                 .validate(token_or_empty().c_str(), token_or_empty().size());

    if (!valid)
    {
      throw AuthError("Authentication failed");
    }
  }
  catch (const std::exception& e)
  {
    // Convert caosFilter exceptions to our error types
    throw AuthError(std::string("Authentication error: ") + e.what());
  }
}
