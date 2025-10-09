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
#include <chrono>
#include <memory>
#include "../Cache.hpp"
#include "generated_queries/Query_Override.hpp"

class Redis final : public IRepository
{
  public:
    Redis(std::unique_ptr<IRepository>&, const sw::redis::ConnectionOptions&, const sw::redis::ConnectionPoolOptions&);

    ~Redis() = default;

    QUERY_OVERRIDE() /* <- from "generated_queries/Query_Override.hpp" */

    // Manually insert your query override here

  private:
    Cache*                        cache;
    std::unique_ptr<IRepository>& database;
    std::unique_ptr<sw::redis::Redis> redis;
};
