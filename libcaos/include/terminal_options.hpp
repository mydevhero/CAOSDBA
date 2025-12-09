#pragma once

#include <config.hpp>
#include <cxxopts.hpp>

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Terminal option management, add and parse options following caos command on terminal
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
class TerminalOptions {
  public:
    TerminalOptions(const TerminalOptions&) = delete;
    TerminalOptions& operator=(const TerminalOptions&) = delete;

    static TerminalOptions& get_instance(int argc = 0, char** argv = nullptr)
    {
      if (!instance_)
      {
          instance_ = std::make_unique<TerminalOptions>(argc, argv);
      }
      return *instance_;
    }

#ifdef CAOS_ENV_TEST
    static void reset()
    {
      instance_.reset();
    }
#endif

    TerminalOptions(int argc, char** argv);
    ~TerminalOptions() = default;

    void parse();
    [[nodiscard]] bool has(const std::string& option) const noexcept;


    template <typename T>
    T get(const std::string& option) const
    {
      if (!result)
      {
        throw std::runtime_error("Options not parsed yet");
      }
      return (*result)[option].as<T>();
    }


  private:
    [[nodiscard]] std::string help() const noexcept;

    TerminalOptions& add_option(const std::string& name, const std::string& description);

    template <typename ValueType>
    TerminalOptions& add_option(const std::string& name, const std::string& description, ValueType&& value)
    {
      options->add_options()(name, description, std::forward<ValueType>(value));
      return *this;
    }

    void read()                          const noexcept;

    int argc;
    char** argv;
    std::unique_ptr<cxxopts::Options> options;
    std::unique_ptr<cxxopts::ParseResult> result;
    static std::unique_ptr<TerminalOptions> instance_;
};
/* -------------------------------------------------------------------------------------------------
 *
 * -----------------------------------------------------------------------------------------------*/
