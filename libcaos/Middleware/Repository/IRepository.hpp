#pragma once

#include <optional>
#include <atomic>
#include <memory>
#include "config.hpp"
#include "IQuery.hpp"
#include "Exception.hpp"
#include <terminal_options.hpp>

#include <arpa/inet.h>                                                                              // Validate IP address
#include <type_traits>

namespace detail                                                                                    // Type Check Utility for std::chrono::duration
{
  template <typename T>                                                                             // Helper to check if T is a specialization of std::chrono::duration
  struct is_chrono_duration : std::false_type {};

  template <typename Rep, typename Period>
  struct is_chrono_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

  template <typename T>
  constexpr bool is_chrono_duration_v = is_chrono_duration<T>::type::value;
}

namespace Policy
{
  class StringIsPresent
  {
    private:
      std::string shortVarName {"Policy::StringIsPresent::shortVarName undefined"};

    public:
      StringIsPresent(const std::string& shortVarName_) : shortVarName(shortVarName_) {}

      void operator()(const std::string& str) const
      {
        // Deadly check ----------------------------------------------------------------------------
        if (str.empty())
        {
          throw std::invalid_argument(this->shortVarName + " empty!");
        }
        // -----------------------------------------------------------------------------------------
      }
  };

  class HostValidator
  {
    private:
      std::string shortVarName {"Policy::HostValidator::shortVarName undefined"};

    public:
      HostValidator(const std::string& shortVarName_) : shortVarName(shortVarName_) {}

      void operator()(const std::string& host) const
      {
        // Deadly check ----------------------------------------------------------------------------
        if (host.empty())
        {
          throw std::invalid_argument(this->shortVarName + " empty!");
        }
        // -----------------------------------------------------------------------------------------



        // Validation ------------------------------------------------------------------------------
        struct sockaddr_in sa4{};
        struct sockaddr_in6 sa6{};

        // inet_pton returns 1 on success, 0 on invalid, -1 on error
        if (inet_pton(AF_INET, host.c_str(), &(sa4.sin_addr)) != 1
            && inet_pton(AF_INET6, host.c_str(), &(sa6.sin6_addr)) != 1)
        {
            throw std::invalid_argument("Invalid Address: " + host);
        }
        // -----------------------------------------------------------------------------------------
      }
  };

  class PortValidator
  {
    private:
      std::string shortVarName {"Policy::PortValidator::shortVarName undefined"};

    public:
      PortValidator(const std::string& shortVarName_) : shortVarName(shortVarName_){}

      void operator()(const std::uint16_t& port) const
      {
        // Deadly check ----------------------------------------------------------------------------
        if(port < unprivileged_port_min || port > unprivileged_port_max)
        {
          throw std::out_of_range(
            this->shortVarName
            + " = "
            + std::to_string(port)
            + " (Allowed values between "
            + std::to_string (unprivileged_port_min)
            + " and "
            + std::to_string (unprivileged_port_max)
            + ")"
          );
        }
        // -----------------------------------------------------------------------------------------
      }
  };

  class ThreadsValidator
  {
    private:
      std::string shortVarName {"Policy::ThreadsValidator::shortVarName undefined"};

    public:
      ThreadsValidator(const std::string& shortVarName_) : shortVarName(shortVarName_){}

      void operator()(std::size_t& threads) const
      {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        if (threads == 0)
        {
          threads = getHwMaxThreads();                                                              // Set thread count to max hardware value
          spdlog::info(
            "{} default value was zero, overriding with max hardware thread count ({})",
            this->shortVarName,
            threads
          );
        }
        // Deadly check ----------------------------------------------------------------------------
        else if (threads < CAOS_DEFAULT_THREADS_LIMIT_MIN || threads > getHwMaxThreads())
#pragma GCC diagnostic pop
        {
          throw std::out_of_range(
            this->shortVarName
            + " = "
            + std::to_string(threads)
            + " (Allowed values between "
            + std::to_string (CAOS_DEFAULT_THREADS_LIMIT_MIN)
            + " and "
            + std::to_string (getHwMaxThreads())
            + ")"
          );
        }
        // -----------------------------------------------------------------------------------------
      }
  };

  template <typename T>
  class InRange
  {
    private:
      std::string shortVarName {"Policy::InRange::shortVarName undefined"};
      T min {0};
      T max {0};


    public:
      InRange(const std::string& shortVarName_, const T& min_, const T& max_)
        : shortVarName(shortVarName_), min(min_), max(max_) {}

      void operator()(const T& configField) const
      {
        // Deadly check --------------------------------------------------------------------------------
        if (configField < this->min || configField > this->max)
        {
          throw std::out_of_range(this->shortVarName + " must be defined and in range between " + std::to_string(this->min) + " and " + std::to_string(this->max) );
        }
        // ---------------------------------------------------------------------------------------------
      };
  };

  template <typename T>
  class NumberAtLeast
  {
    private:
      std::string   shortVarName {"Policy::GreaterOrEqualToZero::shortVarName Undefined"};
      std::uint32_t min {0};

    public:
      NumberAtLeast(const std::string& shortVarName_, const std::uint32_t& min_)
        : shortVarName(shortVarName_), min(min_) {}

      void operator()(const T& configField) const
      {
        // Deadly check ----------------------------------------------------------------------------
        if constexpr (detail::is_chrono_duration_v<T>)
        {
          if (this->min > configField.count())                                                      // std::chrono::duration types
          {
            throw std::out_of_range(shortVarName + " must be at least equal to " + std::to_string(this->min));
          }
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
          if (this->min > configField)                                                              // standard numeric types (int, float, size_t, etc.)
          {
            throw std::out_of_range(shortVarName + " must be at least equal to " + std::to_string(this->min));
          }
        }
        else {
          // Throw a static_assert for unsupported types
          static_assert(detail::is_chrono_duration_v<T> || std::is_arithmetic_v<T>,
                        "T must be an arithmetic type or a std::chrono::duration.");
        }
        // -----------------------------------------------------------------------------------------
      }
  };

  class LogLevelValidator
  {
    private:
      std::string shortVarName {"Policy::LogLevelValidator::shortVarName undefined"};
      LogSeverity& level;

    public:
      LogLevelValidator(const std::string& shortVarName_, LogSeverity& level_)
        : shortVarName(shortVarName_),
          level(level_)
      {}

      void operator()(std::string& name) const
      {
        // Deadly check ----------------------------------------------------------------------------
        if (name.empty())
        {
          throw std::invalid_argument(this->shortVarName + " empty!");
        }
        // -----------------------------------------------------------------------------------------



        // Validation ------------------------------------------------------------------------------
        bool conversionFailed = false;

        try
        {
          this->level = String2LogSeverity(name);
        }
        catch(const std::out_of_range&)
        {
          conversionFailed = true;
        }

        if(conversionFailed)
        {
          throw std::invalid_argument(
            this->shortVarName  +
            R"( error! Allowed log severity is "trace", "debug", "info", "warn", "err", "critical", "off")"
          );
        }
        // -----------------------------------------------------------------------------------------
      }
  };

  /**
   * @brief Validation Policy that performs no operation.
   * Used when no specific validation is required for a configuration field.
   * The implicit validation (e.g., from std::stoll or cxxopts type conversion) is sufficient.
   * @tparam T Type of the configuration field to be received (must be generic).
   */
  template <typename T>
  class NoOpValidator
  {
    public:
      /**
       * @brief The call operator. Executes the validation logic (in this case, none).
       * @param none
       */
      void operator()(const T&) const
      {
        // No operation. No exceptions thrown.
      }
  };
} // End Of namespace Policy

/**
 * @brief Template function to abstract configuration logic (ENV/Options, Validation, Exception Handling).
 * * This function handles I/O (getenv/cxxopts), assignment to the final configuration field, and
 * consistent exception management, leaving the specific validation logic to the Policy (Validator).
 * * @tparam T Final configuration field type (e.g., std::uint16_t, std::string, std::chrono::seconds).
 * @tparam ReadType Intermediate read type (e.g., std::uint32_t for chrono counts). Defaults to T.
 * @tparam Validator Type of the validation function (lambda or functor).
 * @param configField Reference to the field in the configuration struct.
 * @param terminalPtr Pointer to the instance handling cxxopts.
 * @param envName Environment variable name.
 * @param optName Command line option name.
 * @param fieldName Logical name of the field (for logging).
 * @param callerName Full name of the calling function (e.g., "CrowCpp::setPort").
 * @param validator The function/lambda that executes the specific validation logic.
 * @param defaultFinal Final message on exit
 * @param exitOnError If true, calls std::exit(1) for external errors (stoi, cxxopts); otherwise, throw.
 */
template <typename T, typename ReadType = T, typename Validator>
void configureValue(
  T& configField,
  TerminalOptions* terminalPtr,
  const char* const envName,
  const char* const optName,
  const char* const fieldName,
  const std::string& callerName,
  Validator validator,
  const char* defaultFinal,
  bool exitOnError = false)
{
  try
  {
    // ReadType readValue;
    ReadType readValue = ReadType{};

    // 1. READ: Get value from ENV or OPTIONS
    if (const char* valueStr = std::getenv(envName))
    {
      // Use if constexpr to select code block based on ReadType at compile time
      if constexpr (std::is_same_v<ReadType, std::string>)
      {
        readValue = valueStr;
      }
      else if constexpr (std::is_arithmetic_v<ReadType> || std::is_integral_v<ReadType>)
      {
        // Read numeric values using stoll for safety, then cast to ReadType
        readValue = static_cast<ReadType>(std::stoll(valueStr));
      }
      else
      {
        // Compile-time error for unsupported ReadType
        static_assert(std::is_same_v<ReadType, T>, "Unsupported ReadType for configuration.");
      }
    }
    else if (terminalPtr)
    {
      // Read from command line options using cxxopts wrapper
      readValue = terminalPtr->get<ReadType>(optName);
    }
    else
    {
      // No source found (env/options). Assume default get by default of cxxopts
    }

    // 2. ASSIGNMENT: Assign read value to final configField
    if constexpr (detail::is_chrono_duration_v<T>) {
      // 1. Extract the Period (time unit, e.g., std::milli) from the target type T.
      using T_Period = typename T::period;

      // 2. Define the temporary duration using ReadType (the numeric type) and T's Period.
      // Example: std::chrono::duration<std::uint32_t, std::milli>
      using read_duration_t = std::chrono::duration<ReadType, T_Period>;

      // 3. Perform the cast: the count (readValue) is now interpreted in the correct unit (e.g., milliseconds).
      configField = std::chrono::duration_cast<T>(read_duration_t(readValue));
    }
    else
    {
      // Direct assignment for standard types (string, int, etc.)
      configField = readValue;
    }

    // // 2. ASSIGNMENT: Assign read value to final configField
    // if constexpr (detail::is_chrono_duration_v<T>) {
    //   // Generic conversion for all std::chrono::duration types
    //   // Create a temporary duration using ReadType as the count
    //   using read_duration_t = std::chrono::duration<ReadType>;

    //   // Cast the temporary duration to the target duration type T (e.g., seconds to milliseconds)
    //   configField = std::chrono::duration_cast<T>(read_duration_t(readValue));
    // }
    // else
    // {
    //   // Direct assignment for standard types (string, int, etc.)
    //   configField = readValue;
    // }

    // 3. VALIDATION: Execute the specific Validation Policy
    // The validator lambda/functor must throw an exception on failure
    validator(configField);
  }
  // 4. CATCH & LOG: Exception Handling (common logic for all types)
  catch(const std::exception& e)
  {
    spdlog::critical("[{}] : {}", callerName, e.what());

    // Configurable exit policy
    if (exitOnError)
    {
      std::exit(1);
    }
    else
    {
      throw;
    }
  }

  // 5. FINAL: Success Logging

  // Log the actual value of the configured field
  if constexpr (detail::is_chrono_duration_v<T>) {
    // For chrono, log the count value (e.g., 5000 for 5 seconds)
    spdlog::info(defaultFinal, callerName, fieldName, configField.count(), CAOS_ENV);
  } else {
    // Direct logging for standard types
    spdlog::info(defaultFinal, callerName, fieldName, configField, CAOS_ENV);
  }
}


class Utils
{
  protected:
    TerminalOptions*                              terminalPtr           = nullptr           ;
};

class IRepository: public IQuery
{
  public:
    IRepository() = default;
    virtual ~IRepository() = default;
};
