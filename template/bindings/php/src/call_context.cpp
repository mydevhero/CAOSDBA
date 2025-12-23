#include <call_context.hpp>
#include <libcaos.hpp>

namespace {
  // Helper: extract optional string from PHP array
  std::optional<std::string> extract_optional_string(zval* array_zval, const char* key)
  {
    zval* item = nullptr;

    // Check if the key exists in the array
    item = zend_hash_str_find(Z_ARRVAL_P(array_zval), key, strlen(key));

    if (!item)
    {
      return std::nullopt;
    }

    // Convert to string if possible
    if (Z_TYPE_P(item) == IS_STRING)
    {
      // Return even empty strings, caosFilter will handle them.
      return std::string(Z_STRVAL_P(item), Z_STRLEN_P(item));
    }

    // Try to convert other types to string
    zval tmp;
    ZVAL_COPY(&tmp, item);
    convert_to_string(&tmp);

    std::string result(Z_STRVAL(tmp), Z_STRLEN(tmp));
    zval_ptr_dtor(&tmp);
    return result;
  }
}

// ============================================================================
// CallContext Implementation
// ============================================================================

CallContext CallContext::from_php_array(zval* array_zval, const std::string& query_name)
{
  if (!array_zval || Z_TYPE_P(array_zval) != IS_ARRAY)
  {
    throw ValidationError("call_context must be an array", "PARAMETER");
  }

  Data data;

  // Extract token. Empty string is a valid value here.
  data.token = extract_optional_string(array_zval, "token");

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
    // Convert any exception from caosFilter to our AuthError.
    throw AuthError(std::string("Authentication error: ") + e.what());
  }
}
