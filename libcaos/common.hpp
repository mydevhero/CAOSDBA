#pragma once

#include <build_info.h>
#include <thread>
#include <stdexcept>
#include <memory>
#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <logo.hpp>
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
  All         = Repository | CrowCpp
};

inline initFlags operator&(initFlags lhs, initFlags rhs)
{
  using T = std::underlying_type_t<initFlags>;
  return static_cast<initFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline initFlags operator|(initFlags lhs, initFlags rhs) {
    using T = std::underlying_type_t<initFlags>;
    return static_cast<initFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline bool hasFlag(initFlags value, initFlags flag)
{
  return (value & flag) != initFlags::None; // or static_cast<uint8_t>(...) != 0
}

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
