#pragma once

#include <napi.h>
#include <string>
#include <optional>
#include <stdexcept>  // <-- MANCAVA QUESTO! Per std::runtime_error

/**
 * CallContext - Call context for CAOS queries
 *
 * Handles extraction of call metadata from JavaScript object.
 * Delegates all validation to caosFilter.
 */
class CallContext
{
  public:
    // Exceptions for error handling
    class ValidationError : public std::runtime_error  // <-- Richiede #include <stdexcept>
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
     * Create CallContext from N-API JavaScript object
     */
    static CallContext from_napi_object(const Napi::Object& obj, const std::string& query_name);

    /**
     * Apply authentication filters using caosFilter
     * @throws ValidationError or AuthError if validation fails
     */
    void apply_auth_filters(const std::string& query_name) const;

    // Accessors
    const Data& data() const { return data_; }
    bool has_token() const { return data_.has_token(); }

    const std::string& token() const
    {
      if (!data_.token.has_value())
      {
        throw std::runtime_error("Token not available in call context");
      }
      return data_.token.value();
    }

    std::string token_or_empty() const
    {
      return data_.get_token_or_empty();
    }

  private:
    CallContext(Data data) : data_(std::move(data)) {}
    Data data_;
};
