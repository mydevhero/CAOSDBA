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

std::atomic<bool> running = true;













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
    // SPDLOG_TRACE("Init logs");

    // auto default_logger = spdlog::default_logger();

    // if (default_logger) {
    //   SPDLOG_TRACE("Logger already initialized");
    //   return;
    // }

    //  spdlog::shutdown();

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

spdlog::trace("Combined sink created and set it as default");
  }
  catch (const spdlog::spdlog_ex &ex)
  {
    SPDLOG_CRITICAL("Error while log init: {}", ex.what());
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



void Caos::Log::setSeverity()
{
  SPDLOG_TRACE("Setting log severity");

  static constexpr const char* fName = "Caos::Log::setSeverity";

  try
  {
    if (const char* severity_name = std::getenv(CAOS_ENV_LOG_SEVERITY_NAME))
    {
      this->logger.combined.severity.name = severity_name;
    }
    else
    {
      this->logger.combined.severity.name = TerminalOptions::get_instance().get<std::string>(CAOS_OPT_LOG_SEVERITY_NAME);
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
void Caos::PRINT_LOGO() noexcept
{
  std::cout << _logo << "\n\n\n";
}

void Caos::PRINT_VERSION() noexcept
{
  std::cout << "CAOS Version: " << CAOS_VERSION << "\n";
}

void Caos::PRINT_HEADER() noexcept
{
  Caos::PRINT_VERSION();
  std::cout << "\n\n";
}

Caos::Caos(int argc, char* argv[]) : terminalPtr(&TerminalOptions::get_instance(argc, argv))
{
  this->init(initFlags::Repository|initFlags::CrowCpp);
}

Caos::Caos(int argc, char* argv[], initFlags flags) : terminalPtr(&TerminalOptions::get_instance(argc, argv))
{
  this->init(flags);
}


Caos::~Caos()
{
  running.store(false,std::memory_order_release);

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
/* -------------------------------------------------------------------------------------------------
 * class caos
 * -----------------------------------------------------------------------------------------------*/

void Caos::init(initFlags flags)
{
  if ((static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(initFlags::Repository)) != 0)
  {
    this->repository = std::make_unique<Cache>(std::make_unique<Database>());
  }

  if ((static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(initFlags::CrowCpp)) != 0)
  {
#ifdef CAOS_USE_CROWCPP
      this->crowcpp = std::make_unique<CrowCpp>();
#else
#error "Can't provide crowcpp object while CAOS_USE_CROWCPP is OFF in CMakeLists.txt"
#endif
  }


  Caos::PRINT_HEADER();
}
