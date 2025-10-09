/*
 * CAOS - <Cache App On Steroids>
 * Copyright (C) 2025  Alessandro Bianco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For support or inquiries, contact <mydevhero@gmail.com>
 */


#include "include/libcaos.hpp"
#include <optional>
#include <sstream>














/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class Caos::Log
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
const std::unordered_map<std::string, LogSeverity> Caos::Log::LogSeverityMapToString =
{
  { "trace"   , LogSeverity::trace    },
  { "debug"   , LogSeverity::debug    },
  { "info"    , LogSeverity::info     },
  { "warn"    , LogSeverity::warn     },
  { "error"   , LogSeverity::err      },
  { "critical", LogSeverity::critical },
  { "off"     , LogSeverity::off      }
};



constexpr const char* Caos::Log::LogSeverity2String(LogSeverity severity)
{
  if (severity < LogSeverity::n_levels)
  {
    return SeverityChar.at(static_cast<size_t>(severity));
  }

  throw std::out_of_range("Log severity is unnknown");
}



inline LogSeverity Caos::Log::String2LogSeverity(const std::string& severityStr)
{
  auto it = LogSeverityMapToString.find(severityStr);

  if (it != LogSeverityMapToString.end())
  {
    return it->second;
  }

  throw std::out_of_range("Invalid log severity string: " + severityStr);
}



inline void Caos::Log::init()
{
  logger.terminal.pattern = CAOS_LOG_TERMINAL_PATTERN;
  logger.rotating.pattern = CAOS_LOG_ROTATING_PATTERN;
  logger.syslog.pattern   = CAOS_LOG_SYSLOG_PATTERN;

  try
  {
    SPDLOG_TRACE("Init logs");

    auto default_logger = spdlog::default_logger();

    if (default_logger) {
      SPDLOG_TRACE("Logger already initialized");
      return;
    }

    spdlog::init_thread_pool(CAOS_LOG_QUEUE, CAOS_LOG_THREAD_COUNT);

    // Terminal
    logger.terminal.sink= std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    logger.terminal.sink->set_pattern (logger.terminal.pattern);
    SPDLOG_TRACE("Terminal sink created");

    // Rotating file
    logger.rotating.sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(CAOS_LOG_FILE,
                                                             CAOS_LOG_FILE_DIMENSION,
                                                             CAOS_LOG_NUMBER);
    logger.rotating.sink->set_pattern (logger.rotating.pattern);
    SPDLOG_TRACE("Rotating file sink created");

    // Syslog
    logger.syslog.sink= std::make_shared<spdlog::sinks::syslog_sink_mt>(libname, LOG_PID, LOG_USER, true);
    logger.syslog.sink->set_pattern (logger.syslog.pattern);
    SPDLOG_TRACE("Syslog sink created");

    // Combined
    logger.combined.sink =
      std::make_shared<spdlog::async_logger>(
        libname,
        spdlog::sinks_init_list{logger.terminal.sink,logger.rotating.sink,logger.syslog.sink},
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
      );

    logger.combined.sink->set_level(logger.combined.severity.level);

    spdlog::register_logger(logger.combined.sink);
    spdlog::set_default_logger(logger.combined.sink);

    spdlog::flush_every(std::chrono::seconds(3));

    SPDLOG_TRACE("Combined sink created and set it as default");
  }
  catch (const spdlog::spdlog_ex &ex)
  {
    SPDLOG_CRITICAL("Errore nell'inizializzazione del log: {}", ex.what());
    std::exit(1);
  }
}



Caos::Log::Log()
{
  SPDLOG_TRACE("Starting logger");

  this->setSeverity();

  this->init();

  SPDLOG_TRACE("Logger ready");
}



void Caos::Log::setSeverity()                                                                       // Set Log Severity & Severity Name
{
  SPDLOG_TRACE("Setting log severity");

  static constexpr const char* fName = "Caos::Log::setSeverity";

  try
  {
    const char* severity_name = std::getenv(CAOS_ENV_LOG_SEVERITY_NAME);

    if (TerminalOptions::get_instance().has(CAOS_OPT_LOG_SEVERITY_NAME))
    {
      this->logger.combined.severity.name = TerminalOptions::get_instance().get<std::string>(CAOS_OPT_LOG_SEVERITY_NAME);
    }
    else if (severity_name!=nullptr)
    {
      this->logger.combined.severity.name = severity_name;
    }

    // Set by default based on environment
    if (this->logger.terminal.severity.name.empty() || this->logger.syslog.severity.name.empty())
    {
      if (Environment::get_instance().getEnv() == Environment::ENV::dev)
      {
        this->logger.combined.severity.name = CAOS_LOG_SEVERITY_ON_DEV_VALUE;
      }
      else if (Environment::get_instance().getEnv() == Environment::ENV::test)
      {
        this->logger.combined.severity.name = CAOS_LOG_SEVERITY_ON_TEST_VALUE;
      }
      else if (Environment::get_instance().getEnv() == Environment::ENV::production)
      {
        this->logger.combined.severity.name = CAOS_LOG_SEVERITY_ON_PRODUCTION_VALUE;
      }
    }

    SPDLOG_TRACE("Log severity is \"{}\"",this->logger.combined.severity.name);
    this->logger.combined.severity.level = String2LogSeverity(this->logger.combined.severity.name);
  }
  catch(const std::out_of_range& e)
  {
    std::string userMsg = R"(Allowed log severity is "trace", "debug", "info", "warn", "error", "critical", "off")";
    SPDLOG_CRITICAL("[{}] : [{}] : {}", fName, userMsg, e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    SPDLOG_CRITICAL("[{}] : {}", fName, e.what());
    std::exit(1);
  }
}
/* -------------------------------------------------------------------------------------------------
 * class Caos::Log
 * -----------------------------------------------------------------------------------------------*/






/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class implementazione caos
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void Caos::PRINT_LOGO()                 noexcept
{
  std::cout << _logo << "\n";
}

void Caos::PRINT_VERSION()          noexcept
{
  std::cout << "CAOS Version: " << CAOS_VERSION << "\n";
}

void Caos::PRINT_HEADER()               noexcept
{
  Caos::PRINT_VERSION();
  std::cout << "\n\n";
}

Caos::~Caos()
{
#ifdef CAOS_USE_CROWCPP
  if (this->crowcpp != nullptr)
  {
    this->crowcpp.reset();
  }
#endif

  if (this->repository != nullptr)
  {
    this->repository.reset();
  }
}

Caos::Caos(int argc, char* argv[], initFlags flags)
:
  terminalPtr(&TerminalOptions::get_instance(argc, argv)),
  environmentRef(Environment::get_instance())
{

  if (static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(initFlags::Repository))
  {
    this->repository = std::make_unique<Cache>(std::make_unique<Database>());
  }

  if (static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(initFlags::CrowCpp)) {
#ifdef CAOS_USE_CROWCPP
    this->crowcpp = std::make_unique<CrowCpp>();
#else
    spdlog::warn("Init of CrowCpp failed because CAOS_USE_CROWCPP is Off");
#endif
  }

  Caos::PRINT_HEADER();
}
/* -------------------------------------------------------------------------------------------------
 * class caos
 * -----------------------------------------------------------------------------------------------*/
