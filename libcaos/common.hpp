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
#include <libcaos/build_info.h>
#include <thread>
#include <stdexcept>
#include <memory>
#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <cstdint>
#include "config_validation.hpp"
#include "spdlog/spdlog.h"
#include "logo.hpp"
#include <atomic>

static inline std::size_t getHwMaxThreads()
{
  static std::size_t threadsCount = std::thread::hardware_concurrency() * 2;
  return threadsCount;
}

extern std::atomic<bool> running;
enum class initFlags:std::uint8_t {
  None        = 0,
  Repository  = 1 << 0,
  CrowCpp     = 1 << 1,
  PHP_EXT     = 1 << 2,
  All         = Repository | CrowCpp | PHP_EXT
};



using LogSeverity = spdlog::level::level_enum;
enum class LogDestinationSink: std::uint8_t { terminal=0, syslog, EOE };
static constexpr int unprivileged_port_min = CAOS_DEFAULT_UNPRIVILEGED_PORT_MIN;
static constexpr int unprivileged_port_max = CAOS_DEFAULT_UNPRIVILEGED_PORT_MAX;

const std::unordered_map<std::string, LogSeverity> LogSeverityMapToString =
{
  { "trace"   , LogSeverity::trace    },
  { "debug"   , LogSeverity::debug    },
  { "info"    , LogSeverity::info     },
  { "warn"    , LogSeverity::warn     },
  { "err"     , LogSeverity::err      },
  { "critical", LogSeverity::critical },
  { "off"     , LogSeverity::off      }
};

static constexpr std::array<const char*, static_cast<std::size_t>(LogSeverity::n_levels)>
  SeverityChar = { "trace", "debug", "info", "warn", "error", "critical", "off" };


constexpr const char* LogSeverity2String(LogSeverity severity)
{
  if (severity < LogSeverity::n_levels)
  {
    return SeverityChar.at(static_cast<size_t>(severity));
  }

  throw std::out_of_range("Log severity is unnknown");
}



inline LogSeverity String2LogSeverity(std::string& severityStr)
{
  auto it = LogSeverityMapToString.find(severityStr);

  if (it != LogSeverityMapToString.end())
  {
    return it->second;
  }

  // Do not throw error
  // throw std::out_of_range("Invalid log severity string: " + severityStr);
  // Use default severity from config_validation.hpp
  severityStr = CAOS_LOG_SEVERITY;
  return LogSeverityMapToString.at(CAOS_LOG_SEVERITY);
}
