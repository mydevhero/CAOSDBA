#pragma once

#include "generated_queries/AuthConfig.hpp"
#include <random>
#include "spdlog/spdlog.h"

namespace caosFilter
{
  namespace Auth
  {
    class Token {
      private:
        std::string function_name_;
        AuthConfig auth_config_;
        bool requires_auth_ {false};

      public:
        explicit Token(const char* function_name)
          : function_name_(function_name ? function_name : ""),
            requires_auth_(false)
        {
          // 1. Look up the authentication configuration for this function
          auto auth_config_it = QUERY_AUTH_MAP.find(function_name_);
          if (auth_config_it == QUERY_AUTH_MAP.end())
          {
            // Function doesn't require authentication
            requires_auth_ = false;
          }
          else
          {
            requires_auth_ = true;
            auth_config_ = auth_config_it->second;
          }
        }

        bool validate(const char* provided_token, size_t token_length) const
        {
          if (!requires_auth_)
          {
            return true;
          }

          // 2. Get the expected token from environment variable
          const char* expected_token = std::getenv(auth_config_.envVar.c_str());
          if (!expected_token)
          {
            // Environment variable not set
            return false;
          }

          // 3. Compare tokens
          std::string provided_str(provided_token, token_length);
          return provided_str == expected_token;
        }

        [[nodiscard]] bool                requiresAuthentication()  const { return requires_auth_; }
        [[nodiscard]] const std::string&  getFunctionName()         const { return function_name_; }
        [[nodiscard]] const AuthConfig&   getAuthConfig()           const { return auth_config_;   }

        // Static method for fast validation
        static bool validateToken(const char* provided_token, size_t token_length, const char* function_name)
        {
          Token token(function_name);
          return token.validate(provided_token, token_length);
        }

        static std::string generateToken(size_t length = 32)
        {
          const std::string chars =
              "0123456789"
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "abcdefghijklmnopqrstuvwxyz";

          std::random_device rd;
          std::mt19937 generator(rd());
          std::uniform_int_distribution<> distribution(0, chars.size() - 1);

          std::string token;
          token.reserve(length);

          for (size_t i = 0; i < length; ++i)
          {
            token += chars[distribution(generator)];
          }

          return token;
        }
    };
  } // End of namespace Auth
} // End of namespace caosFilter
