#pragma once

// class LogFixture {
// public:
//     LogFixture() {
//       char* argv[] = {(char*)"test", nullptr};
//       TerminalOptions::get_instance(1, argv);
//     }
// };



class LogFixture
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

    LogFixture() = default;
    ~LogFixture() { cleanup(); }

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

TEST_CASE_METHOD(LogFixture, "Log system initializes without crashing [terminal-integration]")
{
  SECTION("Init with trace severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "trace"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::trace);
    REQUIRE(name == "trace");
  }

  SECTION("Init with debug severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "debug"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::debug);
    REQUIRE(name == "debug");
  }

  SECTION("Init with info severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "info"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::info);
    REQUIRE(name == "info");
  }

  SECTION("Init with warn severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "warn"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::warn);
    REQUIRE(name == "warn");
  }

  SECTION("Init with err severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "err"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::err);
    REQUIRE(name == "err");
  }

  SECTION("Init with critical severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "critical"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::critical);
    REQUIRE(name == "critical");
  }

  SECTION("Init with off severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "off"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::off);
    REQUIRE(name == "off");
  }
}

TEST_CASE_METHOD(LogFixture, "Log system initializes without crashing with false value [terminal-integration]")
{
  SECTION("Init with false severity")
  {
    setup({"--" CAOS_LOG_SEVERITY_OPT_NAME, "this_severity_does_not_exists"});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == LogSeverityMapToString.at(CAOS_LOG_SEVERITY));
    REQUIRE(name == CAOS_LOG_SEVERITY);
  }
}

TEST_CASE_METHOD(LogFixture, "Log system initializes without crashing [envireonment-integration]")
{
  SECTION("Init with trace severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "trace", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::trace);
    REQUIRE(name == "trace");

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("Init with debug severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "debug", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::debug);
    REQUIRE(name == "debug");

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("Init with info severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "info", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::info);
    REQUIRE(name == "info");

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("Init with warn severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "warn", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::warn);
    REQUIRE(name == "warn");

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("Init with err severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "err", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::err);
    REQUIRE(name == "err");

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("Init with critical severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "critical", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::critical);
    REQUIRE(name == "critical");

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }

  SECTION("Init with off severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "off", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == spdlog::level::off);
    REQUIRE(name == "off");
  }
}

TEST_CASE_METHOD(LogFixture, "Log system initializes without crashing with false value [environment-integration]")
{
  SECTION("Init with false severity")
  {
    setenv(CAOS_LOG_SEVERITY_ENV_NAME, "this_severity_does_not_exists", 1);
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == LogSeverityMapToString.at(CAOS_LOG_SEVERITY));
    REQUIRE(name == CAOS_LOG_SEVERITY);

    unsetenv(CAOS_LOG_SEVERITY_ENV_NAME);
  }
}

TEST_CASE_METHOD(LogFixture, "Log system initializes without crashing with no value [terminal-environment-integration]")
{
  SECTION("Init with no severity")
  {
    setup({});

    LogSeverity level;
    std::string name;

    REQUIRE_NOTHROW([&level, &name]()
    {
      Caos::Log logger;
      level = logger.getLevel();
      name = logger.getName();
    }());

    REQUIRE(level == LogSeverityMapToString.at(CAOS_LOG_SEVERITY));
    REQUIRE(name == CAOS_LOG_SEVERITY);
  }
}
