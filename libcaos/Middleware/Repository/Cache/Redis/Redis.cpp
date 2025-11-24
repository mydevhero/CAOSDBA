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

#include "Redis.hpp"

#ifdef CAOS_BUILD_EXAMPLES
#include "Query.hpp"
#endif

#ifdef CAOS_CUSTOM_CODE
#include "../src/include/Cache/Redis/Query.hpp"
#endif









/***************************************************************************************************
 *
 *
 * Redis() Constructor/Destructor
 *
 *
 **************************************************************************************************/
Redis::Redis(std::unique_ptr<IRepository>& database_, const sw::redis::ConnectionOptions& connectOpt, const sw::redis::ConnectionPoolOptions& poolOpt)
  : database(database_),
    redis(std::make_unique<sw::redis::Redis>(connectOpt, poolOpt))
{
}
/***************************************************************************************************
 *
 *
 *
 *
 *
 **************************************************************************************************/










void Cache::Pool::setConnectOpt() noexcept
{
  sw::redis::ConnectionOptions options;
  options.host      = this->getHost();
  options.port      = this->getPort ();
  options.db        = this->getIndex();

  if (!this->getPass().empty())
  {
    options.password  = this->getPass();
  }

  this->config.connection_options = options;
}

void Cache::Pool::setPoolOpt() noexcept
{
  sw::redis::ConnectionPoolOptions options;
  options.size                  = this->getPoolSizeMin();
  options.wait_timeout          = this->getPoolWait();
  options.connection_lifetime   = this->getPoolConnectionLifetime();
  options.connection_idle_time  = this->getPoolConnectionIdletime();

  this->config.pool_options = options;
}

const sw::redis::ConnectionOptions      Cache::Pool::getConnectOpt() const noexcept { return this->config.connection_options; }
const sw::redis::ConnectionPoolOptions  Cache::Pool::getPoolOpt()    const noexcept { return this->config.pool_options;       }
