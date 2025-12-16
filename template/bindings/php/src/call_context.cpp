#include <call_context.hpp>
#include <libcaos.hpp>
#include <unordered_map>
#include <algorithm>

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
      if (Z_STRLEN_P(item) == 0)
      {
        return std::nullopt;
      }
      return std::string(Z_STRVAL_P(item), Z_STRLEN_P(item));
    }

    // Try to convert other types to string
    zval tmp;
    ZVAL_COPY(&tmp, item);
    convert_to_string(&tmp);

    if (Z_STRLEN(tmp) == 0)
    {
      zval_ptr_dtor(&tmp);
      return std::nullopt;
    }

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

  // TOKEN - Optional field
  data.token = extract_optional_string(array_zval, "token");

  return CallContext(std::move(data));
}

// ============================================================================
// Validator Implementations
// ============================================================================

bool CallContext::TokenValidator::should_validate(const CallContext& ctx) const
{
  // Only validate if token is present in context
  return ctx.has_token();
}

bool CallContext::TokenValidator::validate(const CallContext& ctx,
                                           const std::string& query_name) const
{
  try
  {
    // Use existing CAOS authentication system
    // Token is guaranteed to be present when this is called (checked by should_validate)
    return caosFilter::Auth::Token(query_name.c_str())
        .validate(ctx.token().c_str(), ctx.token().size());
  }
  catch (const std::exception& e)
  {
    throw AuthError(std::string("Token validation failed: ") + e.what());
  }
}

// ============================================================================
// Validator Registry (updated validation logic)
// ============================================================================

CallContext::ValidatorRegistry& CallContext::ValidatorRegistry::instance()
{
  static ValidatorRegistry registry;
  return registry;
}

void CallContext::validate(const std::string& query_name) const
{
  auto& registry = ValidatorRegistry::instance();

  // Collect all validators for this query
  std::vector<const Validator*> validators;

  // Global validators
  for (const auto& validator : registry.global_validators)
  {
    validators.push_back(validator.get());
  }

  // Query-specific validators
  if (auto it = registry.query_validators.find(query_name);
      it != registry.query_validators.end())
  {
    for (const auto& validator : it->second)
    {
      validators.push_back(validator.get());
    }
  }

  // Sort by priority
  std::sort(validators.begin(), validators.end(),
            [](const Validator* a, const Validator* b){
    return a->priority() < b->priority();
  });

  // Execute validation in order, but only for validators that should run
  for (const auto& validator : validators)
  {
    if (validator->should_validate(*this))
    {
      if (!validator->validate(*this, query_name))
      {
        throw ValidationError(
              std::string("Validation failed by ") + validator->name(),
              "VALIDATION"
              );
      }
    }
  }
}

void CallContext::register_validator(std::unique_ptr<Validator> validator)
{
  auto& registry = ValidatorRegistry::instance();
  registry.global_validators.push_back(std::move(validator));
}

void CallContext::register_validator(const std::string& query_pattern,
                                     std::unique_ptr<Validator> validator)
{
  auto& registry = ValidatorRegistry::instance();
  registry.query_validators[query_pattern].push_back(std::move(validator));
}

void CallContext::clear_validators()
{
  auto& registry = ValidatorRegistry::instance();
  registry.global_validators.clear();
  registry.query_validators.clear();
}
