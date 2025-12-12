#pragma once

#include <Python.h>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_map>

/**
 * CallContext - Call context for CAOS queries
 *
 * Handles extraction and validation of call metadata from Python dictionary.
 * Token is optional - validation only occurs if token is present.
 */
class CallContext {
  public:
    // Specific exceptions
    class ValidationError : public std::runtime_error {
      public:
        ValidationError(const std::string& what, const std::string& type = "VALIDATION")
          : std::runtime_error(what), error_type_(type) {}

        const std::string& error_type() const { return error_type_; }

      private:
        std::string error_type_;
    };

    class AuthError : public ValidationError {
      public:
        AuthError(const std::string& what) : ValidationError(what, "AUTH") {}
    };

    // Data structure with optional token
    struct Data {
        std::optional<std::string> token;  // Optional authentication token

        // Check if token is present and non-empty
        bool has_token() const {
          return token.has_value() && !token->empty();
        }

        // Get token value or empty string
        std::string get_token_or_empty() const {
          return token.value_or("");
        }
    };

    /**
     * Create CallContext from Python dictionary
     * @throws ValidationError if dict is malformed
     */
    static CallContext from_python_dict(PyObject* dict, const std::string& query_name);

    /**
     * Context validator - Base interface
     */
    class Validator {
      public:
        virtual ~Validator() = default;

        /**
         * Validate context for specific query
         * @return true if valid, false otherwise
         * @throws ValidationError with details
         */
        virtual bool validate(const CallContext& ctx, const std::string& query_name) const = 0;

        /**
         * Check if this validator should run for the given context
         */
        virtual bool should_validate(const CallContext& ctx) const { return true; }

        /**
         * Execution priority (lower = earlier)
         */
        virtual int priority() const { return 100; }

        /**
         * Identifier name for debugging
         */
        virtual std::string name() const = 0;
    };

    /**
     * Token validator - Uses caosFilter::Auth::Token
     * Only validates if token is present in context
     */
    class TokenValidator : public Validator {
      public:
        bool validate(const CallContext& ctx, const std::string& query_name) const override;
        bool should_validate(const CallContext& ctx) const override;
        int priority() const override { return 10; }
        std::string name() const override { return "TokenValidator"; }
    };

    // Accessors
    const Data& data() const { return data_; }

    // Check if token is present
    bool has_token() const { return data_.has_token(); }

    // Get token (throws if not present)
    const std::string& token() const {
      if (!data_.token.has_value()) {
        throw std::runtime_error("Token not available in call context");
      }
      return data_.token.value();
    }

    // Get token or empty string
    std::string token_or_empty() const {
      return data_.get_token_or_empty();
    }

    /**
     * Execute all registered validators
     * Only runs validators that should_validate() returns true
     * @throws ValidationError if any validator fails
     */
    void validate(const std::string& query_name) const;

    /**
     * Register a global validator (valid for all queries)
     */
    static void register_validator(std::unique_ptr<Validator> validator);

    /**
     * Register a validator for specific query
     */
    static void register_validator(const std::string& query_pattern,
                                   std::unique_ptr<Validator> validator);

    /**
     * Clear all registered validators
     */
    static void clear_validators();

  private:
    CallContext(Data data) : data_(std::move(data)) {}

    Data data_;

    // Validator registry
    struct ValidatorRegistry {
        std::vector<std::unique_ptr<Validator>> global_validators;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Validator>>> query_validators;

        static ValidatorRegistry& instance();
    };
};
