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

#pragma once

#include <libcaos/config.hpp>
#include <terminal_options.hpp>
#include <environment.hpp>

#include "IRepository.hpp"
#include "Cache/Cache.hpp"
#include "Database/Database.hpp"

#ifdef CAOS_USE_CROWCPP
#include "CrowCpp.hpp"
#endif

#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog/sinks/syslog_sink.h"

#include <array>

inline initFlags operator|(initFlags flag1, initFlags flag2)
{
  return static_cast<initFlags>(
    static_cast<std::uint8_t>(flag1) | static_cast<std::uint8_t>(flag2)
  );
}

inline initFlags operator&(initFlags flag1, initFlags flag2)
{
  return static_cast<initFlags>(
    static_cast<std::uint8_t>(flag1) & static_cast<std::uint8_t>(flag2)
  );
}

inline initFlags operator|=(initFlags& flag1, initFlags flag2)
{
  flag1 = flag1 | flag2;
  return flag1;
}

inline bool operator!(initFlags flag1)
{
  return static_cast<std::uint8_t>(flag1) == 0;
}

class Caos
{
  public:

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * Log configuration & log utilities manager
     * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    class Log
    {
      private:
        struct LoggerNameDestinationSeverity
        {
          LogSeverity level;
          std::string name;
        };

        template<typename T>
        struct LoggerNameDestination {
            std::shared_ptr<T> sink;
            LoggerNameDestinationSeverity severity;
            std::string pattern;
        };

        struct Logger
        {
            LoggerNameDestination<spdlog::sinks::stdout_color_sink_mt> terminal;
            LoggerNameDestination<spdlog::sinks::rotating_file_sink_mt> rotating;
            LoggerNameDestination<spdlog::sinks::syslog_sink_mt> syslog;
            LoggerNameDestination<spdlog::async_logger> combined;
        };

        Logger logger;

        void setSeverity();

        static constexpr std::array<const char*, static_cast<std::size_t>(LogSeverity::n_levels)>
          SeverityChar = { "trace", "debug", "info", "warn", "error", "critical", "off" };

        static const std::unordered_map<std::string, LogSeverity> LogSeverityMapToString;
        static constexpr inline const char* LogSeverity2String(LogSeverity severity);
        static inline LogSeverity String2LogSeverity(const std::string& severityStr);

        inline void init();
      public:
         Log();
    };
    /* ---------------------------------------------------------------------------------------------
     *
     * -------------------------------------------------------------------------------------------*/


    const TerminalOptions*    terminalPtr;
    const Environment&        environmentRef;
    const Log                 log;
    std::unique_ptr<Cache>    repository;

#ifdef CAOS_USE_CROWCPP
    std::unique_ptr<CrowCpp>  crowcpp;
#endif

    Caos(int argc,
         char* argv[],
         initFlags = initFlags::Repository
    );

    static void PRINT_LOGO()                  noexcept;
    static void PRINT_VERSION()               noexcept;
    static void PRINT_HEADER()                noexcept;
    void readTerminalOption()           const noexcept;

    ~Caos();
};
