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
#include <exception>
#include <string>
#include "libcaos/config.hpp"

namespace repository
{
  class broken_connection : public std::exception
  {
  private:
    std::string message_;

  public:
    explicit broken_connection(const std::string& what_arg)
      : message_(what_arg) {}

    explicit broken_connection(const char* what_arg)
      : message_(what_arg) {}

    virtual const char* what() const noexcept override {
      return message_.c_str();
    }

    virtual ~broken_connection() = default;
  };
}
