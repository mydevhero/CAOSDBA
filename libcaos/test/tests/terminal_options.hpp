#pragma once

#include "../common.hpp"
#include "../include/terminal_options.hpp"
#include <vector>
#include <catch2/catch_test_macros.hpp>

class TerminalOptionsFixture
{
  public:
    void setup(const std::vector<std::string>& args)
    {
      cleanup();

      std::vector<char*> argv;
      std::string program_name = "caos";
      argv.push_back(program_name.data());

      std::vector<std::string> mutable_args = args;
      for (auto& arg : mutable_args) {
        argv.push_back(arg.data());
      }

      terminalPtr = &TerminalOptions::get_instance(argv.size(), argv.data());
    }

    TerminalOptionsFixture() = default;
    ~TerminalOptionsFixture() { cleanup(); }

    TerminalOptions* operator->() { return terminalPtr; }
    const TerminalOptions* operator->() const { return terminalPtr; }

    TerminalOptions& operator*() { return *terminalPtr; }
    const TerminalOptions& operator*() const { return *terminalPtr; }

  private:
    void cleanup()
    {
      if (this->terminalPtr)
      {
        this->terminalPtr->reset();
      }
    }

    TerminalOptions* terminalPtr = nullptr;
};

TEST_CASE_METHOD(TerminalOptionsFixture, "Parses valid log severity options [terminal]")
{
  SECTION("trace severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "trace"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "trace");
    REQUIRE(String2LogSeverity(str) == LogSeverity::trace);
  }

  SECTION("debug severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "debug"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "debug");
    REQUIRE(String2LogSeverity(str) == LogSeverity::debug);
  }

  SECTION("info severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "info"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "info");
    REQUIRE(String2LogSeverity(str) == LogSeverity::info);
  }

  SECTION("warn severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "warn"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "warn");
    REQUIRE(String2LogSeverity(str) == LogSeverity::warn);
  }

  SECTION("err severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "err"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "err");
    REQUIRE(String2LogSeverity(str) == LogSeverity::err);
  }

  SECTION("critical severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "critical"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "critical");
    REQUIRE(String2LogSeverity(str) == LogSeverity::critical);
  }

  SECTION("off severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "off"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "off");
    REQUIRE(String2LogSeverity(str) == LogSeverity::off);
  }
}

TEST_CASE_METHOD(TerminalOptionsFixture, "Uses default log severity when not provided [terminal]")
{
  SECTION("default severity when no option passed")
  {
    setup({});

    REQUIRE_FALSE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == CAOS_LOG_SEVERITY); // CAOS_LOG_SEVERITY default defined in config_validation.hpp
    REQUIRE(String2LogSeverity(str));
  }
}

TEST_CASE_METHOD(TerminalOptionsFixture, "Check false log severity [terminal]")
{
  SECTION("false severity should be rejected")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "this_severity_does_not_exists"});
    REQUIRE((*this)->has(CAOS_LOG_SEVERITY_OPT_NAME));

    auto str = (*this)->get<std::string>(CAOS_LOG_SEVERITY_OPT_NAME);

    REQUIRE(str == "this_severity_does_not_exists");
    LogSeverity logSeverity = String2LogSeverity(str);  // Should return the default value from config_validation.hpp

    REQUIRE(str == CAOS_LOG_SEVERITY);

    auto expected_severity = LogSeverityMapToString.at(CAOS_LOG_SEVERITY);
    REQUIRE(logSeverity == expected_severity);
  }
}

TEST_CASE("Parses valid log severity options [environment]")
{
  SECTION("trace severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "trace", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "trace");
    REQUIRE(String2LogSeverity(str) == LogSeverity::trace);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("debug severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "debug", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "debug");
    REQUIRE(String2LogSeverity(str) == LogSeverity::debug);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("info severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "info", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "info");
    REQUIRE(String2LogSeverity(str) == LogSeverity::info);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("warn severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "warn", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "warn");
    REQUIRE(String2LogSeverity(str) == LogSeverity::warn);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("err severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "err", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "err");
    REQUIRE(String2LogSeverity(str) == LogSeverity::err);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("critical severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "critical", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "critical");
    REQUIRE(String2LogSeverity(str) == LogSeverity::critical);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("off severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "off", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE_FALSE(str.empty());
    REQUIRE(str == "off");
    REQUIRE(String2LogSeverity(str) == LogSeverity::off);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }
}

TEST_CASE("Check false log severity [environment]")
{
  SECTION("false severity should be rejected")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "this_severity_does_not_exists", 1);
    std::string str = std::getenv(CAOS_LOG_SEVERITY_ENV_NAME);
    REQUIRE(str == "this_severity_does_not_exists");
    LogSeverity logSeverity = String2LogSeverity(str);  // Should return the default value from config_validation.hpp
    REQUIRE(str == CAOS_LOG_SEVERITY);
    auto expected_severity = LogSeverityMapToString.at(CAOS_LOG_SEVERITY);
    REQUIRE(logSeverity == expected_severity);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }
}
