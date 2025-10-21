#pragma once

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

TEST_CASE_METHOD(TerminalOptionsFixture, "Parses log severity option", "[terminal]")
{
  setup({"--" CAOS_OPT_LOG_SEVERITY_NAME, "debug"});

  REQUIRE((*this)->has(CAOS_OPT_LOG_SEVERITY_NAME));
  REQUIRE((*this)->get<std::string>(CAOS_OPT_LOG_SEVERITY_NAME) == "debug");
}

// TEST_CASE_METHOD(TerminalOptionsFixture, "Parses environment option", "[terminal]")
// {
//   setup({"--" CAOS_OPT_APP_ENV_NAME, "release"});

//   REQUIRE((*this)->has(CAOS_OPT_APP_ENV_NAME));
//   REQUIRE((*this)->get<std::string>(CAOS_OPT_APP_ENV_NAME) == "release");
// }
