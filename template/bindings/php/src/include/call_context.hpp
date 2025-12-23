#pragma once

#include <php.h>
#include <string>
#include <optional>
#include <stdexcept>

/**
 * CallContext - Call context for CAOS queries (PHP version)
 *
 * Rewritten based on Node.js architecture as the source of truth.
 * Solely extracts data. All validation logic is delegated to caosFilter.
 */
class CallContext
{
  public:
    // Exceptions for error handling
    class ValidationError : public std::runtime_error
    {
      public:
        ValidationError(const std::string& what, const std::string& type = "VALIDATION")
          : std::runtime_error(what), error_type_(type) {}

        const std::string& error_type() const { return error_type_; }

      private:
        std::string error_type_;
    };

    class AuthError : public ValidationError
    {
      public:
        AuthError(const std::string& what) : ValidationError(what, "AUTH") {}
    };

    struct Data
    {
        std::optional<std::string> token;  // Optional authentication token

        bool has_token() const
        {
          return token.has_value() && !token->empty();
        }

        std::string get_token_or_empty() const
        {
          return token.value_or("");
        }
    };

    /**
     * Create CallContext from PHP array (zval)
     * @throws ValidationError if input is not a PHP array
     */
    static CallContext from_php_array(zval* array_zval, const std::string& query_name);

    /**
     * Apply authentication filters using caosFilter.
     * This is the single validation point, always called.
     * @throws ValidationError or AuthError if caosFilter validation fails
     */
    void apply_auth_filters(const std::string& query_name) const;

    // Accessors
    const Data& data() const { return data_; }
    bool has_token() const { return data_.has_token(); }

    // Get token (throws if not present)
    const std::string& token() const
    {
      if (!data_.token.has_value())
      {
        throw std::runtime_error("Token not available in call context");
      }
      return data_.token.value();
    }

    // Get token or empty string. Primary method for validation.
    std::string token_or_empty() const
    {
      return data_.get_token_or_empty();
    }

  private:
    CallContext(Data data) : data_(std::move(data)) {}
    Data data_;
};
