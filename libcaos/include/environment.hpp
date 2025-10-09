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

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Environment (vars) configuration manager
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
class Environment
{
  public:
    enum class ENV:std::uint8_t { dev=0, test, production, unmanaged, EOE };                        // EOE = End Of Enum

  private:
    const TerminalOptions* terminalPtr;

    ENV   env;
    std::string name;

    void setEnv();

    static constexpr std::array<const char*, static_cast<std::size_t>(ENV::EOE)>
      ENVChar = { "dev", "test", "production", "unmanaged" };

    static const std::unordered_map<std::string, ENV> ENVMapToString;
    static constexpr const char* ENV2String(ENV env);
    static inline ENV String2ENV(const std::string& envStr);

  public:
    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;

    static Environment& get_instance()
    {
      static Environment instance;
      return instance;
    }

    Environment();

    [[nodiscard]] ENV         getEnv()  const noexcept;
    [[nodiscard]] std::string getName() const noexcept;
};
/* ---------------------------------------------------------------------------------------------
 *
 * -------------------------------------------------------------------------------------------*/
