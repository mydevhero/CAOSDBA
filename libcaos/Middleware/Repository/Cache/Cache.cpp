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

#include "Cache.hpp"
#include "Query.hpp"

#ifdef CAOS_USE_CACHE_REDIS
#include "Redis/Redis.hpp"
#endif

#include <arpa/inet.h>

constexpr const char* defaultFinal = "{} : Setting database {} to {} in {}  environment";


#ifdef CAOS_USE_CACHE_REDIS
std::unique_ptr<IRepository> Cache::Pool::init(std::unique_ptr<IRepository>& database)
{
  return std::make_unique<Redis>(database, this->getConnectOpt(), this->getPoolOpt());
}
#endif



Cache::Cache(std::unique_ptr<IRepository> database__)
  : database_(std::move(database__)),
    pool(std::make_unique<Pool>()),
    cache(pool->init(database_))
{
}

Cache::~Cache()
{
  spdlog::trace("Destroying Cache");
  this->database_.reset();
  this->pool.reset();
  this->cache.reset();
  spdlog::info("Cache destroyed");
};


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setUser()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setUser()
{
  static constexpr const char* fName = "Cache::Pool::setUser";

  try
  {
    // Set primary cache user for production environment -------------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEUSER_NAME))
    {
      this->config.user = this->terminalPtr->get<std::string>(CAOS_OPT_CACHEUSER_NAME);
    }
    else if (const char* env_addr = std::getenv(CAOS_ENV_CACHEUSER_NAME))
    {
      this->config.user = env_addr;
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEUSER)
      this->config.user = CAOS_DEFAULT_CACHEUSER;
#endif
    }
    // ---------------------------------------------------------------------------------------------


    // Set alternative cache user for test or dev environment --------------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEUSER_ON_DEV_OR_TEST)
      this->config.user = "" CAOS_DEFAULT_CACHEUSER_ON_DEV_OR_TEST;
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }




  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "user",
               this->config.user,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setUser()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPass()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPass()
{
  static constexpr const char* fName = "Cache::Pool::setPass";

  try
  {
    // Set primary cache password for production environment ---------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPASS_NAME))
    {
      this->config.pass = this->terminalPtr->get<std::string>(CAOS_OPT_CACHEPASS_NAME);
    }
    else if (const char* env_addr = std::getenv(CAOS_ENV_CACHEPASS_NAME))
    {
      this->config.pass = env_addr;
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPASS)
      this->config.pass = CAOS_DEFAULT_CACHEPASS;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache password for test or dev environment ----------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPASS_ON_DEV_OR_TEST)
      this->config.pass = "" CAOS_DEFAULT_CACHEPASS_ON_DEV_OR_TEST;
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
                  fName,
                  "password",
                  "*******",
                  this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPass()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setHost()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setHost()
{
  static constexpr const char* fName = "Cache::Pool::setHost";

  try
  {
    // Set primary cache host for production environment -------------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEHOST_NAME))
    {
      this->config.host = this->terminalPtr->get<std::string>(CAOS_OPT_CACHEHOST_NAME);
    }
    else if (const char* env_addr = std::getenv(CAOS_ENV_CACHEHOST_NAME))
    {
      this->config.host = env_addr;
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEHOST)
      this->config.host = CAOS_DEFAULT_CACHEHOST;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache host for test or dev environment --------------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEHOST_ON_DEV_OR_TEST)
      this->config.host = "" CAOS_DEFAULT_CACHEHOST_ON_DEV_OR_TEST;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Validation ----------------------------------------------------------------------------------
    struct sockaddr_in sa4={};
    struct sockaddr_in6 sa6={};

    if (inet_pton(AF_INET, this->config.host.c_str(), &(sa4.sin_addr)) == 0
        && inet_pton(AF_INET6, this->config.host.c_str(), &(sa6.sin6_addr)) == 0)
    {
      throw std::invalid_argument("Invalid Address: " + this->config.host);
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "host",
               this->config.host,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setHost()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPort()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPort()
{
  static constexpr const char* fName = "Cache::Pool::setPort";

  try
  {
    // Set primary cache port for production environment -------------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPORT_NAME))
    {
      this->config.port = this->terminalPtr->get<std::uint16_t>(CAOS_OPT_CACHEPORT_NAME);
    }
    else if (const char* port_str = std::getenv(CAOS_ENV_CACHEPORT_NAME))
    {
      this->config.port = static_cast<std::uint16_t>(std::stoi(port_str));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPORT) && (CAOS_DEFAULT_CACHEPORT + 0) > 0
      this->config.port = static_cast<std::uint16_t>(CAOS_DEFAULT_CACHEPORT);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache port for test or dev environment --------------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPORT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPORT_ON_DEV_OR_TEST + 0) > 0
      this->config.port = static_cast<std::uint16_t>(CAOS_DEFAULT_CACHEPORT_ON_DEV_OR_TEST);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Validation ----------------------------------------------------------------------------------
    if(this->config.port < unprivileged_port_min || this->config.port > unprivileged_port_max)
    {
      throw std::out_of_range("TCP Port out of range: " + std::to_string(this->config.port));
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::out_of_range& e)
  {
    spdlog::critical("[{}] : [TCP Port range from {} to {}] : {}",fName, unprivileged_port_min, unprivileged_port_max, e.what());
    throw;
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    throw;
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "port",
               this->config.port,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPort()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setClientName()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setClientName()
{
  static constexpr const char* fName = "Cache::Pool::setClientName";

  try
  {
    // Set primary cache client name for production environment ------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHECLIENTNAME_NAME))
    {
      this->config.clientname = this->terminalPtr->get<std::string>(CAOS_OPT_CACHECLIENTNAME_NAME);
    }
    else if (const char* env_val = std::getenv(CAOS_ENV_CACHECLIENTNAME_NAME))
    {
      this->config.clientname = env_val;
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHECLIENTNAME)
      this->config.clientname = CAOS_DEFAULT_CACHECLIENTNAME;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache client name for test or dev environment -------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHECLIENTNAME_ON_DEV_OR_TEST)
        this->config.clientname = "" CAOS_DEFAULT_CACHECLIENTNAME_ON_DEV_OR_TEST;
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
                  fName,
                  "ClientName",
                  this->config.clientname,
                  this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setClientName()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setIndex()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setIndex()
{
  static constexpr const char* fName = "Cache::Pool::setIndex";

  try
  {
    // Set primary cache index for production environment ------------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEINDEX_NAME))
    {

      this->config.index = this->terminalPtr->get<std::uint8_t>(CAOS_OPT_CACHEINDEX_NAME);
    }
    else if (const char* index = std::getenv(CAOS_ENV_CACHEINDEX_NAME))
    {
      this->config.index = static_cast<std::uint8_t>(std::stoi(index));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEINDEX) && (CAOS_DEFAULT_CACHEINDEX + 0) > 0
      this->config.index = static_cast<std::uint8_t>(CAOS_DEFAULT_CACHEINDEX);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache index for test or dev environment -------------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEINDEX_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEINDEX_ON_DEV_OR_TEST + 0) > 0
      this->config.index = static_cast<std::uint8_t>(CAOS_DEFAULT_CACHEINDEX_ON_DEV_OR_TEST);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Check cacheindex ----------------------------------------------------------------------------
    if (this->config.index > 15)
    {
      this->config.index = 0;
      spdlog::info("CACHEINDEX hardcoded to {}", this->config.index);
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }

  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "index",
               this->config.index,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setIndex()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setCommandTimeout()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setCommandTimeout()
{
  static constexpr const char* fName = "Cache::Pool::setCommandTimeout";

  try
  {
    // Set primary cache commandtimeout for production environment ---------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHECOMMANDTIMEOUT_NAME))
    {
      this->config.commandtimeout = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHECOMMANDTIMEOUT_NAME));
    }
    else if (const char* commandtimeout = std::getenv(CAOS_ENV_CACHECOMMANDTIMEOUT_NAME))
    {
      this->config.commandtimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(commandtimeout)));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHECOMMANDTIMEOUT) && (CAOS_DEFAULT_CACHECOMMANDTIMEOUT + 0) > 0
      this->config.commandtimeout = std::chrono::milliseconds(CAOS_DEFAULT_CACHECOMMANDTIMEOUT);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache commandtimeout for test or dev environment ----------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHECOMMANDTIMEOUT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHECOMMANDTIMEOUT_ON_DEV_OR_TEST + 0) > 0
      this->config.commandtimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(CAOS_DEFAULT_CACHECOMMANDTIMEOUT_ON_DEV_OR_TEST));
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "commandtimeout",
               static_cast<std::uint32_t>(this->config.commandtimeout.count()),
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setCommandTimeout()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPoolSizeMin()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPoolSizeMin()
{
  static constexpr const char* fName = "Cache::Pool::setPoolSizeMin";

  try
  {
    // Set primary cache poolsizemin for production environment ------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLSIZEMIN_NAME))
    {
      this->config.poolsizemin = this->terminalPtr->get<std::size_t>(CAOS_OPT_CACHEPOOLSIZEMIN_NAME);
    }
    else if (const char* poolsizemin = std::getenv(CAOS_ENV_CACHEPOOLSIZEMIN_NAME))
    {
      this->config.poolsizemin = static_cast<std::size_t>(std::stoi(poolsizemin));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPOOLSIZEMIN) && (CAOS_DEFAULT_CACHEPOOLSIZEMIN + 0) > 0
      this->config.poolsizemin = static_cast<std::size_t>(CAOS_DEFAULT_CACHEPOOLSIZEMIN);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache poolsizemin for test or dev environment -------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPOOLSIZEMIN_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLSIZEMIN_ON_DEV_OR_TEST + 0) > 0
      this->config.poolsizemin = static_cast<std::size_t>(CAOS_DEFAULT_CACHEPOOLSIZEMIN_ON_DEV_OR_TEST);
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }

  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "poolsizemin",
               this->config.poolsizemin,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolSizeMin()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPoolSizeMax()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPoolSizeMax()
{
  static constexpr const char* fName = "Cache::Pool::setPoolSizeMax";

  try
  {
    // Set primary cache poolsizemax for production environment ------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLSIZEMAX_NAME))
    {
      this->config.poolsizemax = this->terminalPtr->get<std::size_t>(CAOS_OPT_CACHEPOOLSIZEMAX_NAME);
    }
    else if (const char* poolsizemax = std::getenv(CAOS_ENV_CACHEPOOLSIZEMAX_NAME))
    {
      this->config.poolsizemax = static_cast<std::size_t>(std::stoi(poolsizemax));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPOOLSIZEMAX) && (CAOS_DEFAULT_CACHEPOOLSIZEMAX + 0) > 0
      this->config.poolsizemax = CAOS_DEFAULT_CACHEPOOLSIZEMAX;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache poolsizemax for test or dev environment -------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPOOLSIZEMAX_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLSIZEMAX_ON_DEV_OR_TEST + 0) > 0
      this->config.poolsizemax = static_cast<std::size_t>(CAOS_DEFAULT_CACHEPOOLSIZEMAX_ON_DEV_OR_TEST);
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }

  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "poolsizemax",
               this->config.poolsizemax,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolSizeMax()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPoolWait()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPoolWait()
{
  static constexpr const char* fName = "Cache::Pool::setPoolWait";

  try
  {
    // Set primary cache poolwait for production environment ---------------------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLWAIT_NAME))
    {
      this->config.poolwait = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLWAIT_NAME));
    }
    else if (const char* poolwait = std::getenv(CAOS_ENV_CACHEPOOLWAIT_NAME))
    {
      this->config.poolwait = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(poolwait)));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPOOLWAIT) && (CAOS_DEFAULT_CACHEPOOLWAIT + 0) > 0
      this->config.poolwait = std::chrono::milliseconds(CAOS_DEFAULT_CACHEPOOLWAIT);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache poolwait for test or dev environment ----------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPOOLWAIT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLWAIT_ON_DEV_OR_TEST + 0) > 0
      this->config.poolwait =std::chrono::milliseconds(static_cast<std::uint32_t>(CAOS_DEFAULT_CACHEPOOLWAIT_ON_DEV_OR_TEST));
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "poolwait",
               static_cast<uint32_t>(this->config.poolwait.count()),
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolWait()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPoolConnectionTimeout()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPoolConnectionTimeout()
{
  static constexpr const char* fName = "Cache::Pool::setPoolConnectionTimeout()";

  try
  {
    // Set primary cache poolconnectiontimeout for production environment --------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLCONNECTIONTIMEOUT_NAME))
    {
      this->config.poolconnectiontimeout = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLCONNECTIONTIMEOUT_NAME));
    }
    else if (const char* poolconnectiontimeout = std::getenv(CAOS_ENV_CACHEPOOLCONNECTIONTIMEOUT_NAME))
    {
      this->config.poolconnectiontimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(poolconnectiontimeout)));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT + 0) > 0
      this->config.poolconnectiontimeout = std::chrono::milliseconds(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache poolconnectiontimeout for test or dev environment ---------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT_ON_DEV_OR_TEST + 0) > 0
      this->config.poolconnectiontimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT_ON_DEV_OR_TEST)));
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "poolconnectiontimeout",
               static_cast<uint32_t>(this->config.poolconnectiontimeout.count()),
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolConnectionTimeout()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPoolConnectionLifetime()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPoolConnectionLifetime()
{
  static constexpr const char* fName = "Cache::Pool::setPoolConnectionLifetime()";

  try
  {
    // Set primary cache poolconnectionlifetime for production environment -------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLCONNECTIONLIFETIME_NAME))
    {
      this->config.poolconnectionlifetime = std::chrono::seconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLCONNECTIONLIFETIME_NAME));
    }
    else if (const char* poolconnectionlifetime = std::getenv(CAOS_ENV_CACHEPOOLCONNECTIONLIFETIME_NAME))
    {
      this->config.poolconnectionlifetime = std::chrono::seconds(static_cast<std::chrono::seconds>(std::stoi(poolconnectionlifetime)));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME + 0) > 0
      this->config.poolconnectionlifetime = std::chrono::seconds(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache poolconnectionlifetime for test or dev environment --------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME_ON_DEV_OR_TEST + 0) > 0
      this->config.poolconnectionlifetime = std::chrono::seconds(static_cast<std::chrono::seconds>(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME_ON_DEV_OR_TEST));
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "poolconnectionlifetime",
               static_cast<uint32_t>(this->config.poolconnectionlifetime.count()),
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolConnectionLifetime()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setPoolConnectionIdletime()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setPoolConnectionIdletime()
{
  static constexpr const char* fName = "Cache::Pool::setPoolConnectionIdletime()";

  try
  {
    // Set primary cache poolconnectionidletime for production environment -------------------------
    if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLCONNECTIONIDLETIME_NAME))
    {
      this->config.poolconnectionidletime = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLCONNECTIONIDLETIME_NAME));
    }
    else if (const char* poolconnectionidletime = std::getenv(CAOS_ENV_CACHEPOOLCONNECTIONIDLETIME_NAME))
    {
      this->config.poolconnectionidletime = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(poolconnectionidletime)));
    }
    else
    {
#if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME + 0) > 0
      this->config.poolconnectionidletime = std::chrono::milliseconds(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative cache poolconnectionidletime for test or dev environment --------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME_ON_DEV_OR_TEST + 0) > 0
      this->config.poolconnectionidletime = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME_ON_DEV_OR_TEST)));
#endif
    }
    // ---------------------------------------------------------------------------------------------
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}",fName,e.what());
    std::exit(1);
  }



  // Final -----------------------------------------------------------------------------------------
  spdlog::info(defaultFinal,
               fName,
               "poolconnectionidletime",
               static_cast<uint32_t>(this->config.poolconnectionidletime.count()),
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolConnectionIdletime()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------









const std::string&                Cache::Pool::getUser()                    const noexcept { return this->config.user;                    }
const std::string&                Cache::Pool::getPass()                    const noexcept { return this->config.pass;                    }
const std::string&                Cache::Pool::getHost()                    const noexcept { return this->config.host;                    }
const std::uint16_t&              Cache::Pool::getPort()                    const noexcept { return this->config.port;                    }
const std::string&                Cache::Pool::getClientName()              const noexcept { return this->config.clientname;              }
const std::uint8_t&               Cache::Pool::getIndex()                   const noexcept { return this->config.index;                   }
const std::chrono::milliseconds&  Cache::Pool::getCommandTimeout()          const noexcept { return this->config.commandtimeout;          }
const std::size_t&                Cache::Pool::getPoolSizeMin()             const noexcept { return this->config.poolsizemin;             }
const std::size_t&                Cache::Pool::getPoolSizeMax()             const noexcept { return this->config.poolsizemax;             }
const std::chrono::milliseconds&  Cache::Pool::getPoolWait()                const noexcept { return this->config.poolwait;                }
const std::chrono::milliseconds&  Cache::Pool::getPoolConnectionTimeout()   const noexcept { return this->config.poolconnectiontimeout;   }
const std::chrono::seconds&       Cache::Pool::getPoolConnectionLifetime()  const noexcept { return this->config.poolconnectionlifetime;  }
const std::chrono::milliseconds&  Cache::Pool::getPoolConnectionIdletime()  const noexcept { return this->config.poolconnectionidletime;  }












// /*
//  * CAOS - <Cache App On Steroids>
//  * Copyright (C) 2025  Alessandro Bianco
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  *
//  * For support or inquiries, contact <mydevhero@gmail.com>
//  */

// #include "Cache.hpp"
// #include "Query.hpp"

// #ifdef CAOS_USE_CACHE_REDIS
// #include "Redis/Redis.hpp"
// #endif

// #include <arpa/inet.h>

// constexpr const char* defaultFinal = "{} : Setting database {} to {} in {}  environment";






// Cache::Cache(std::unique_ptr<IRepository> database__)
//   : database_(std::move(database__)),
//     config({})
// {
//   if (this->terminalPtr==nullptr)
//   {
//     this->terminalPtr     = &TerminalOptions::get_instance();
//   }

//   if (this->environmentRef==nullptr)
//   {
//     this->environmentRef  = &Environment::get_instance();
//   }

//   this->setUser()                   ;
//   this->setPass()                   ;
//   this->setHost()                   ;
//   this->setPort()                   ;
//   this->setClientName()             ;
//   this->setIndex()                  ;
//   this->setCommandTimeout()         ;

//   this->setPoolSizeMin()            ;
//   this->setPoolSizeMax()            ;
//   if (this->getPoolSizeMin() > this->getPoolSizeMax ())
//   {
//     throw std::out_of_range("CACHEPOOLSIZEMIN > CACHEPOOLSIZEMAX");
//   }

//   this->setPoolWait()               ;
//   this->setPoolConnectionTimeout()  ;
//   this->setPoolConnectionLifetime() ;
//   this->setPoolConnectionIdletime() ;

// #ifdef CAOS_USE_CACHE_REDIS
//   this->setConnectOpt()             ;
//   this->setPoolOpt()                ;

//   this->cache = std::make_unique<Redis>(this);
// #endif
// }

// Cache::~Cache()
// {
//   spdlog::trace("Destroying Cache");
//   this->database_.reset();
//   this->cache.reset();
//   spdlog::info("Cache destroyed");
// };


// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setUser()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setUser()
// {
//   static constexpr const char* fName = "Cache::setUser";

//   try
//   {
//     // Set primary cache user for production environment -------------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEUSER_NAME))
//     {
//       this->config.user = this->terminalPtr->get<std::string>(CAOS_OPT_CACHEUSER_NAME);
//     }
//     else if (const char* env_addr = std::getenv(CAOS_ENV_CACHEUSER_NAME))
//     {
//       this->config.user = env_addr;
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEUSER)
//       this->config.user = CAOS_DEFAULT_CACHEUSER;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------


//     // Set alternative cache user for test or dev environment --------------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEUSER_ON_DEV_OR_TEST)
//       this->config.user = "" CAOS_DEFAULT_CACHEUSER_ON_DEV_OR_TEST;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }




//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "user",
//                this->config.user,
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setUser()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPass()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPass()
// {
//   static constexpr const char* fName = "Cache::setPass";

//   try
//   {
//     // Set primary cache password for production environment ---------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPASS_NAME))
//     {
//       this->config.pass = this->terminalPtr->get<std::string>(CAOS_OPT_CACHEPASS_NAME);
//     }
//     else if (const char* env_addr = std::getenv(CAOS_ENV_CACHEPASS_NAME))
//     {
//       this->config.pass = env_addr;
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPASS)
//       this->config.pass = CAOS_DEFAULT_CACHEPASS;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache password for test or dev environment ----------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPASS_ON_DEV_OR_TEST)
//       this->config.pass = "" CAOS_DEFAULT_CACHEPASS_ON_DEV_OR_TEST;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                   fName,
//                   "password",
//                   "*******",
//                   this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPass()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setHost()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setHost()
// {
//   static constexpr const char* fName = "Cache::setHost";

//   try
//   {
//     // Set primary cache host for production environment -------------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEHOST_NAME))
//     {
//       this->config.host = this->terminalPtr->get<std::string>(CAOS_OPT_CACHEHOST_NAME);
//     }
//     else if (const char* env_addr = std::getenv(CAOS_ENV_CACHEHOST_NAME))
//     {
//       this->config.host = env_addr;
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEHOST)
//       this->config.host = CAOS_DEFAULT_CACHEHOST;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache host for test or dev environment --------------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEHOST_ON_DEV_OR_TEST)
//       this->config.host = "" CAOS_DEFAULT_CACHEHOST_ON_DEV_OR_TEST;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Validation ----------------------------------------------------------------------------------
//     struct sockaddr_in sa4={};
//     struct sockaddr_in6 sa6={};

//     if (inet_pton(AF_INET, this->config.host.c_str(), &(sa4.sin_addr)) == 0
//         && inet_pton(AF_INET6, this->config.host.c_str(), &(sa6.sin6_addr)) == 0)
//     {
//       throw std::invalid_argument("Invalid Address: " + this->config.host);
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "host",
//                this->config.host,
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setHost()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPort()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPort()
// {
//   static constexpr const char* fName = "Cache::setPort";

//   try
//   {
//     // Set primary cache port for production environment -------------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPORT_NAME))
//     {
//       this->config.port = this->terminalPtr->get<std::uint16_t>(CAOS_OPT_CACHEPORT_NAME);
//     }
//     else if (const char* port_str = std::getenv(CAOS_ENV_CACHEPORT_NAME))
//     {
//       this->config.port = static_cast<std::uint16_t>(std::stoi(port_str));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPORT) && (CAOS_DEFAULT_CACHEPORT + 0) > 0
//       this->config.port = static_cast<std::uint16_t>(CAOS_DEFAULT_CACHEPORT);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache port for test or dev environment --------------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPORT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPORT_ON_DEV_OR_TEST + 0) > 0
//       this->config.port = static_cast<std::uint16_t>(CAOS_DEFAULT_CACHEPORT_ON_DEV_OR_TEST);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Validation ----------------------------------------------------------------------------------
//     if(this->config.port < unprivileged_port_min || this->config.port > unprivileged_port_max)
//     {
//       throw std::out_of_range("TCP Port out of range: " + std::to_string(this->config.port));
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::out_of_range& e)
//   {
//     spdlog::critical("[{}] : [TCP Port range from {} to {}] : {}",fName, unprivileged_port_min, unprivileged_port_max, e.what());
//     throw;
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     throw;
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "port",
//                this->config.port,
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPort()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setClientName()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setClientName()
// {
//   static constexpr const char* fName = "Cache::setClientName";

//   try
//   {
//     // Set primary cache client name for production environment ------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHECLIENTNAME_NAME))
//     {
//       this->config.clientname = this->terminalPtr->get<std::string>(CAOS_OPT_CACHECLIENTNAME_NAME);
//     }
//     else if (const char* env_val = std::getenv(CAOS_ENV_CACHECLIENTNAME_NAME))
//     {
//       this->config.clientname = env_val;
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHECLIENTNAME)
//       this->config.clientname = CAOS_DEFAULT_CACHECLIENTNAME;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache client name for test or dev environment -------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHECLIENTNAME_ON_DEV_OR_TEST)
//         this->config.clientname = "" CAOS_DEFAULT_CACHECLIENTNAME_ON_DEV_OR_TEST;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                   fName,
//                   "ClientName",
//                   this->config.clientname,
//                   this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setClientName()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setIndex()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setIndex()
// {
//   static constexpr const char* fName = "Cache::setIndex";

//   try
//   {
//     // Set primary cache index for production environment ------------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEINDEX_NAME))
//     {

//       this->config.index = this->terminalPtr->get<std::uint8_t>(CAOS_OPT_CACHEINDEX_NAME);
//     }
//     else if (const char* index = std::getenv(CAOS_ENV_CACHEINDEX_NAME))
//     {
//       this->config.index = static_cast<std::uint8_t>(std::stoi(index));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEINDEX) && (CAOS_DEFAULT_CACHEINDEX + 0) > 0
//       this->config.index = static_cast<std::uint8_t>(CAOS_DEFAULT_CACHEINDEX);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache index for test or dev environment -------------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEINDEX_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEINDEX_ON_DEV_OR_TEST + 0) > 0
//       this->config.index = static_cast<std::uint8_t>(CAOS_DEFAULT_CACHEINDEX_ON_DEV_OR_TEST);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Check cacheindex ----------------------------------------------------------------------------
//     if (this->config.index > 15)
//     {
//       this->config.index = 0;
//       spdlog::info("CACHEINDEX hardcoded to {}", this->config.index);
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }

//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "index",
//                this->config.index,
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setIndex()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setCommandTimeout()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setCommandTimeout()
// {
//   static constexpr const char* fName = "Cache::setCommandTimeout";

//   try
//   {
//     // Set primary cache commandtimeout for production environment ---------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHECOMMANDTIMEOUT_NAME))
//     {
//       this->config.commandtimeout = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHECOMMANDTIMEOUT_NAME));
//     }
//     else if (const char* commandtimeout = std::getenv(CAOS_ENV_CACHECOMMANDTIMEOUT_NAME))
//     {
//       this->config.commandtimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(commandtimeout)));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHECOMMANDTIMEOUT) && (CAOS_DEFAULT_CACHECOMMANDTIMEOUT + 0) > 0
//       this->config.commandtimeout = std::chrono::milliseconds(CAOS_DEFAULT_CACHECOMMANDTIMEOUT);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache commandtimeout for test or dev environment ----------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHECOMMANDTIMEOUT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHECOMMANDTIMEOUT_ON_DEV_OR_TEST + 0) > 0
//       this->config.commandtimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(CAOS_DEFAULT_CACHECOMMANDTIMEOUT_ON_DEV_OR_TEST));
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "commandtimeout",
//                static_cast<std::uint32_t>(this->config.commandtimeout.count()),
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setCommandTimeout()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPoolSizeMin()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPoolSizeMin()
// {
//   static constexpr const char* fName = "Cache::setPoolSizeMin";

//   try
//   {
//     // Set primary cache poolsizemin for production environment ------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLSIZEMIN_NAME))
//     {
//       this->config.poolsizemin = this->terminalPtr->get<std::size_t>(CAOS_OPT_CACHEPOOLSIZEMIN_NAME);
//     }
//     else if (const char* poolsizemin = std::getenv(CAOS_ENV_CACHEPOOLSIZEMIN_NAME))
//     {
//       this->config.poolsizemin = static_cast<std::size_t>(std::stoi(poolsizemin));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLSIZEMIN) && (CAOS_DEFAULT_CACHEPOOLSIZEMIN + 0) > 0
//       this->config.poolsizemin = static_cast<std::size_t>(CAOS_DEFAULT_CACHEPOOLSIZEMIN);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache poolsizemin for test or dev environment -------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLSIZEMIN_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLSIZEMIN_ON_DEV_OR_TEST + 0) > 0
//       this->config.poolsizemin = static_cast<std::size_t>(CAOS_DEFAULT_CACHEPOOLSIZEMIN_ON_DEV_OR_TEST);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }

//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "poolsizemin",
//                this->config.poolsizemin,
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPoolSizeMin()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPoolSizeMax()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPoolSizeMax()
// {
//   static constexpr const char* fName = "Cache::setPoolSizeMax";

//   try
//   {
//     // Set primary cache poolsizemax for production environment ------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLSIZEMAX_NAME))
//     {
//       this->config.poolsizemax = this->terminalPtr->get<std::size_t>(CAOS_OPT_CACHEPOOLSIZEMAX_NAME);
//     }
//     else if (const char* poolsizemax = std::getenv(CAOS_ENV_CACHEPOOLSIZEMAX_NAME))
//     {
//       this->config.poolsizemax = static_cast<std::size_t>(std::stoi(poolsizemax));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLSIZEMAX) && (CAOS_DEFAULT_CACHEPOOLSIZEMAX + 0) > 0
//       this->config.poolsizemax = CAOS_DEFAULT_CACHEPOOLSIZEMAX;
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache poolsizemax for test or dev environment -------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLSIZEMAX_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLSIZEMAX_ON_DEV_OR_TEST + 0) > 0
//       this->config.poolsizemax = static_cast<std::size_t>(CAOS_DEFAULT_CACHEPOOLSIZEMAX_ON_DEV_OR_TEST);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }

//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "poolsizemax",
//                this->config.poolsizemax,
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPoolSizeMax()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPoolWait()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPoolWait()
// {
//   static constexpr const char* fName = "Cache::setPoolWait";

//   try
//   {
//     // Set primary cache poolwait for production environment ---------------------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLWAIT_NAME))
//     {
//       this->config.poolwait = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLWAIT_NAME));
//     }
//     else if (const char* poolwait = std::getenv(CAOS_ENV_CACHEPOOLWAIT_NAME))
//     {
//       this->config.poolwait = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(poolwait)));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLWAIT) && (CAOS_DEFAULT_CACHEPOOLWAIT + 0) > 0
//       this->config.poolwait = std::chrono::milliseconds(CAOS_DEFAULT_CACHEPOOLWAIT);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache poolwait for test or dev environment ----------------------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLWAIT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLWAIT_ON_DEV_OR_TEST + 0) > 0
//       this->config.poolwait =std::chrono::milliseconds(static_cast<std::uint32_t>(CAOS_DEFAULT_CACHEPOOLWAIT_ON_DEV_OR_TEST));
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "poolwait",
//                static_cast<uint32_t>(this->config.poolwait.count()),
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPoolWait()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPoolConnectionTimeout()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPoolConnectionTimeout()
// {
//   static constexpr const char* fName = "Cache::setPoolConnectionTimeout()";

//   try
//   {
//     // Set primary cache poolconnectiontimeout for production environment --------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLCONNECTIONTIMEOUT_NAME))
//     {
//       this->config.poolconnectiontimeout = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLCONNECTIONTIMEOUT_NAME));
//     }
//     else if (const char* poolconnectiontimeout = std::getenv(CAOS_ENV_CACHEPOOLCONNECTIONTIMEOUT_NAME))
//     {
//       this->config.poolconnectiontimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(poolconnectiontimeout)));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT + 0) > 0
//       this->config.poolconnectiontimeout = std::chrono::milliseconds(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache poolconnectiontimeout for test or dev environment ---------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT_ON_DEV_OR_TEST + 0) > 0
//       this->config.poolconnectiontimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(CAOS_DEFAULT_CACHEPOOLCONNECTIONTIMEOUT_ON_DEV_OR_TEST)));
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "poolconnectiontimeout",
//                static_cast<uint32_t>(this->config.poolconnectiontimeout.count()),
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPoolConnectionTimeout()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPoolConnectionLifetime()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPoolConnectionLifetime()
// {
//   static constexpr const char* fName = "Cache::setPoolConnectionLifetime()";

//   try
//   {
//     // Set primary cache poolconnectionlifetime for production environment -------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLCONNECTIONLIFETIME_NAME))
//     {
//       this->config.poolconnectionlifetime = std::chrono::seconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLCONNECTIONLIFETIME_NAME));
//     }
//     else if (const char* poolconnectionlifetime = std::getenv(CAOS_ENV_CACHEPOOLCONNECTIONLIFETIME_NAME))
//     {
//       this->config.poolconnectionlifetime = std::chrono::seconds(static_cast<std::chrono::seconds>(std::stoi(poolconnectionlifetime)));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME + 0) > 0
//       this->config.poolconnectionlifetime = std::chrono::seconds(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache poolconnectionlifetime for test or dev environment --------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME_ON_DEV_OR_TEST + 0) > 0
//       this->config.poolconnectionlifetime = std::chrono::seconds(static_cast<std::chrono::seconds>(CAOS_DEFAULT_CACHEPOOLCONNECTIONLIFETIME_ON_DEV_OR_TEST));
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "poolconnectionlifetime",
//                static_cast<uint32_t>(this->config.poolconnectionlifetime.count()),
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPoolConnectionLifetime()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------










// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// // Init of Cache::setPoolConnectionIdletime()
// // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Cache::setPoolConnectionIdletime()
// {
//   static constexpr const char* fName = "Cache::setPoolConnectionIdletime()";

//   try
//   {
//     // Set primary cache poolconnectionidletime for production environment -------------------------
//     if (this->terminalPtr->has(CAOS_OPT_CACHEPOOLCONNECTIONIDLETIME_NAME))
//     {
//       this->config.poolconnectionidletime = std::chrono::milliseconds(this->terminalPtr->get<std::uint32_t>(CAOS_OPT_CACHEPOOLCONNECTIONIDLETIME_NAME));
//     }
//     else if (const char* poolconnectionidletime = std::getenv(CAOS_ENV_CACHEPOOLCONNECTIONIDLETIME_NAME))
//     {
//       this->config.poolconnectionidletime = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(poolconnectionidletime)));
//     }
//     else
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME + 0) > 0
//       this->config.poolconnectionidletime = std::chrono::milliseconds(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME);
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------



//     // Set alternative cache poolconnectionidletime for test or dev environment --------------------
//     if (this->isDevOrTestEnv())
//     {
// #if defined(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME_ON_DEV_OR_TEST) && (CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME_ON_DEV_OR_TEST + 0) > 0
//       this->config.poolconnectionidletime = std::chrono::milliseconds(static_cast<std::chrono::milliseconds>(std::stoi(CAOS_DEFAULT_CACHEPOOLCONNECTIONIDLETIME_ON_DEV_OR_TEST)));
// #endif
//     }
//     // ---------------------------------------------------------------------------------------------
//   }
//   catch(const std::invalid_argument& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::critical("[{}] : {}",fName,e.what());
//     std::exit(1);
//   }



//   // Final -----------------------------------------------------------------------------------------
//   spdlog::info(defaultFinal,
//                fName,
//                "poolconnectionidletime",
//                static_cast<uint32_t>(this->config.poolconnectionidletime.count()),
//                this->environmentRef->getName());
// }
// // -------------------------------------------------------------------------------------------------
// // End of Cache::setPoolConnectionIdletime()
// // -------------------------------------------------------------------------------------------------
// // -------------------------------------------------------------------------------------------------









// const std::string&                Cache::getUser()                    const noexcept { return this->config.user;                    }
// const std::string&                Cache::getPass()                    const noexcept { return this->config.pass;                    }
// const std::string&                Cache::getHost()                    const noexcept { return this->config.host;                    }
// const std::uint16_t&              Cache::getPort()                    const noexcept { return this->config.port;                    }
// const std::string&                Cache::getClientName()              const noexcept { return this->config.clientname;              }
// const std::uint8_t&               Cache::getIndex()                   const noexcept { return this->config.index;                   }
// const std::chrono::milliseconds&  Cache::getCommandTimeout()          const noexcept { return this->config.commandtimeout;          }
// const std::size_t&                Cache::getPoolSizeMin()             const noexcept { return this->config.poolsizemin;             }
// const std::size_t&                Cache::getPoolSizeMax()             const noexcept { return this->config.poolsizemax;             }
// const std::chrono::milliseconds&  Cache::getPoolWait()                const noexcept { return this->config.poolwait;                }
// const std::chrono::milliseconds&  Cache::getPoolConnectionTimeout()   const noexcept { return this->config.poolconnectiontimeout;   }
// const std::chrono::seconds&       Cache::getPoolConnectionLifetime()  const noexcept { return this->config.poolconnectionlifetime;  }
// const std::chrono::milliseconds&  Cache::getPoolConnectionIdletime()  const noexcept { return this->config.poolconnectionidletime;  }
