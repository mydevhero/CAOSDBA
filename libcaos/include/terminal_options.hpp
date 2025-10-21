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
#include <cxxopts.hpp>

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Terminal option management, add and parse options following caos command on terminal
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
class TerminalOptions {
  public:
    TerminalOptions(const TerminalOptions&) = delete;
    TerminalOptions& operator=(const TerminalOptions&) = delete;

    static TerminalOptions& get_instance(int argc = 0, char** argv = nullptr)
    {
      if (!instance_)
      {
          instance_ = std::make_unique<TerminalOptions>(argc, argv);
      }
      return *instance_;
    }

#ifdef CAOS_ENV_TEST
    static void reset()
    {
      instance_.reset();
    }
#endif

    TerminalOptions(int argc, char** argv);
    ~TerminalOptions() = default;

    void parse();
    [[nodiscard]] bool has(const std::string& option) const noexcept;


    template <typename T>
    T get(const std::string& option) const
    {
      if (!result)
      {
        throw std::runtime_error("Options not parsed yet");
      }
      return (*result)[option].as<T>();
    }


  private:
    [[nodiscard]] std::string help() const noexcept;

    TerminalOptions& add_option(const std::string& name, const std::string& description);

    template <typename ValueType>
    TerminalOptions& add_option(const std::string& name, const std::string& description, ValueType&& value)
    {
      options->add_options()(name, description, std::forward<ValueType>(value));
      return *this;
    }

    void read()                          const noexcept;

    int argc;
    char** argv;
    std::unique_ptr<cxxopts::Options> options;
    std::unique_ptr<cxxopts::ParseResult> result;
    static std::unique_ptr<TerminalOptions> instance_;
};
/* -------------------------------------------------------------------------------------------------
 *
 * -----------------------------------------------------------------------------------------------*/
