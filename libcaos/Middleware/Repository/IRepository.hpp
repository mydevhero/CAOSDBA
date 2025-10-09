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
#include <optional>
#include <atomic>
#include <memory>
#include "libcaos/config.hpp"
#include "IQuery.hpp"
#include "Exception.hpp"
#include <terminal_options.hpp>
#include <environment.hpp>

class Utils
{
  public:
    [[nodiscard]] bool isDevOrTestEnv() const noexcept { return this->environmentRef->getEnv() == Environment::ENV::dev || this->environmentRef->getEnv() == Environment::ENV::test; }

  protected:
    TerminalOptions*                              terminalPtr           = nullptr           ;
    Environment*                                  environmentRef        = nullptr           ;
};

class IRepository: public IQuery
{
  public:
    std::atomic<bool>                                   running_{true}                    ;

    IRepository() = default;
    virtual ~IRepository() = default;
};
