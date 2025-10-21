#pragma once
#include <atomic>

extern std::atomic<bool> running;
enum class initFlags:std::uint8_t {
  None        = 0,
  Repository  = 1 << 0,
  CrowCpp     = 1 << 1,
  All         = Repository | CrowCpp
};



using LogSeverity = spdlog::level::level_enum;
enum class LogDestinationSink: std::uint8_t { terminal=0, syslog, EOE };
static constexpr int unprivileged_port_min = DEFAULT_UNPRIVILEGED_PORT_MIN;
static constexpr int unprivileged_port_max = DEFAULT_UNPRIVILEGED_PORT_MAX;
