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




constexpr const char* defaultFinalLog = "{} : Setting Caos::Log {} to {} in {} environment";








/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class Caos::Log
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
Caos::Log::Log()
{
  this->setSeverity();

  spdlog::set_level(spdlog::level::trace);

  logger.terminal.pattern = CAOS_LOG_TERMINAL_PATTERN;
  logger.rotating.pattern = CAOS_LOG_ROTATING_PATTERN;
  logger.syslog.pattern   = CAOS_LOG_SYSLOG_PATTERN;

  static std::size_t logCount = 0;
  std::string loggerName = libname + std::to_string(logCount++);


  try
  {
    spdlog::init_thread_pool(CAOS_LOG_QUEUE, CAOS_LOG_THREAD_COUNT);

    // Terminal
    logger.terminal.sink= std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    logger.terminal.sink->set_pattern (logger.terminal.pattern);

    // Rotating file
    logger.rotating.sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(CAOS_LOG_FILE,
                                                             CAOS_LOG_FILE_DIMENSION,
                                                             CAOS_LOG_NUMBER);
    logger.rotating.sink->set_pattern (logger.rotating.pattern);

    // Syslog
    logger.syslog.sink= std::make_shared<spdlog::sinks::syslog_sink_mt>(loggerName, LOG_PID, LOG_USER, true);
    logger.syslog.sink->set_pattern (logger.syslog.pattern);

    // Combined
    logger.combined.sink =
      std::make_shared<spdlog::async_logger>(
        loggerName,
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
    throw;
  }
}

void Caos::Log::setSeverity()
{
  const char* fName     = "Caos::Log::setSeverity"                  ;
  const char* fieldName = "CAOS_LOG_SEVERITY"                       ;
  using       dataType  = std::string                               ;

  Policy::LogLevelValidator validator(
    fieldName,
    this->logger.combined.severity.level
  )                                                                 ;

  configureValue<dataType>(
    this->logger.combined.severity.name,                            // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_LOG_SEVERITY_ENV_NAME,                                     // envName
    CAOS_LOG_SEVERITY_OPT_NAME,                                     // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinalLog,
    false                                                           // exitOnError
  );
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
#warning "Can't provide crowcpp object while CAOS_USE_CROWCPP is OFF in CMakeLists.txt"
#endif
  }


  if ((static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(initFlags::PHP_EXT)) != 0)
  {
#ifdef CAOS_BUILD_PHP_EXTENSION
    // caosFilter::Auth::Token::setAutoToken();
#else
#warning "Can't provide PHP methods while CAOS_BUILD_PHP_EXTENSION is OFF in CMakeLists.txt"
#endif
  }

  // Caos::PRINT_HEADER();
}
