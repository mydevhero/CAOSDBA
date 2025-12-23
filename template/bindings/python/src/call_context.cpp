#include "call_context.hpp"
#include <libcaos.hpp>

namespace {
  // Helper: extract optional string from Python dict
  std::optional<std::string> extract_optional_string(PyObject* dict, const char* key)
  {
    PyObject* obj = PyDict_GetItemString(dict, key);
    if (!obj || !PyUnicode_Check(obj))
    {
      return std::nullopt;
    }

    const char* str = PyUnicode_AsUTF8(obj);
    if (!str)
    {
      return std::nullopt;
    }
    // Return even empty strings, caosFilter will handle them
    return std::string(str);
  }
}

// ============================================================================
// CallContext Implementation
// ============================================================================

CallContext CallContext::from_python_dict(PyObject* dict, const std::string& query_name)
{
  if (!PyDict_Check(dict))
  {
    throw ValidationError("call_context must be a dictionary", "PARAMETER");
  }

  Data data;

  // TOKEN - Optional field (empty string is valid)
  data.token = extract_optional_string(dict, "token");

  return CallContext(std::move(data));
}

void CallContext::apply_auth_filters(const std::string& query_name) const
{
  try
  {
    // Delegate ALL validation to caosFilter.
    // Pass token_or_empty() (could be empty string). caosFilter knows
    // QUERY_AUTH_MAP and decides internally if a token is required.
    bool valid = caosFilter::Auth::Token(query_name.c_str())
                 .validate(token_or_empty().c_str(), token_or_empty().size());

    if (!valid)
    {
      throw AuthError("Authentication failed");
    }
  }
  catch (const std::exception& e)
  {
    // Convert any exception from caosFilter to our AuthError
    throw AuthError(std::string("Authentication error: ") + e.what());
  }
}
