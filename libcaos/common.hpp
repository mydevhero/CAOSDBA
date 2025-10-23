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
#include <libcaos/version.hpp>
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
  All         = Repository | CrowCpp
};



using LogSeverity = spdlog::level::level_enum;
enum class LogDestinationSink: std::uint8_t { terminal=0, syslog, EOE };
static constexpr int unprivileged_port_min = CAOS_DEFAULT_UNPRIVILEGED_PORT_MIN;
static constexpr int unprivileged_port_max = CAOS_DEFAULT_UNPRIVILEGED_PORT_MAX;
