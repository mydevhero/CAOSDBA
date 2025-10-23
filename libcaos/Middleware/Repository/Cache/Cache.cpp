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

constexpr const char* defaultFinal = "{} : Setting cache {} to {} in {} environment";





/***************************************************************************************************
 *
 *
 * Cache::Cache() Constructor/Destructor
 *
 *
 **************************************************************************************************/
Cache::Cache(std::unique_ptr<IRepository> db_)
  : database_(std::move(db_)),
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
/***************************************************************************************************
 *
 *
 *
 *
 *
 **************************************************************************************************/










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::setUser()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Cache::Pool::setUser()
{
  const char* fName     = "Cache::Pool::setUser"                    ;
  const char* fieldName = "CACHEUSER"                               ;
  using dataType        = std::string                               ;

  Policy::NoOpValidator<dataType> noOpValidator                     ;

  configureValue<dataType>(
    this->config.user,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEUSER_ENV_NAME,                                        // envName
    CAOS_CACHEUSER_OPT_NAME,                                        // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    noOpValidator,                                                  // validator in namespace Policy - no validation
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPass"                    ;
  const char* fieldName = "CACHEPASS"                               ;
  using dataType        = std::string                               ;

  Policy::NoOpValidator<dataType> noOpValidator                     ;

  configureValue<dataType>(
    this->config.pass,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPASS_ENV_NAME,                                        // envName
    CAOS_CACHEPASS_OPT_NAME,                                        // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    noOpValidator,                                                  // validator in namespace Policy - no validation
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setHost"                    ;
  const char* fieldName = "CACHEHOST"                               ;
  using dataType        = std::string                               ;

  Policy::HostValidator validator(fieldName)                        ;

  configureValue<dataType>(
    this->config.host,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEHOST_ENV_NAME,                                        // envName
    CAOS_CACHEHOST_OPT_NAME,                                        // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPort"                    ;
  const char* fieldName = "CACHEPORT"                               ;
  using dataType        = std::uint16_t                             ;

  Policy::PortValidator validator("CACHEPORT")                      ;

  configureValue<dataType>(
    this->config.port,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPORT_ENV_NAME,                                        // envName
    CAOS_CACHEPORT_OPT_NAME,                                        // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setClientName"              ;
  const char* fieldName = "CACHECLIENTNAME"                         ;
  using dataType        = std::string                               ;

  Policy::NoOpValidator<dataType> noOpValidator                     ;

  configureValue<dataType>(
    this->config.clientname,                                        // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHECLIENTNAME_ENV_NAME,                                  // envName
    CAOS_CACHECLIENTNAME_OPT_NAME,                                  // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    noOpValidator,                                                  // validator in namespace Policy - no validation
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setIndex"                   ;
  const char* fieldName = "CACHEINDEX"                              ;
  using dataType        = std::uint8_t                              ;

  Policy::InRange<dataType> validator(
    fieldName,
    CAOS_CACHEINDEX_LIMIT_MIN,
    CAOS_CACHEINDEX_LIMIT_MAX)                                      ;

  configureValue<dataType>(
    this->config.index,                                             // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEINDEX_ENV_NAME,                                       // envName
    CAOS_CACHEINDEX_OPT_NAME,                                       // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setCommandTimeout"          ;
  const char* fieldName = "CACHECOMMANDTIMEOUT"                     ;
  using dataType        = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHECOMMANDTIMEOUT_LIMIT_MIN)                             ;

  configureValue<dataType, std::uint32_t>(
    this->config.commandtimeout,                                    // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHECOMMANDTIMEOUT_ENV_NAME,                              // envName
    CAOS_CACHECOMMANDTIMEOUT_OPT_NAME,                              // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPoolSizeMin"             ;
  const char* fieldName = "CACHEPOOLSIZEMIN"                        ;
  using dataType        = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHEPOOLSIZEMIN_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.poolsizemin,                                       // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPOOLSIZEMIN_ENV_NAME,                                 // envName
    CAOS_CACHEPOOLSIZEMIN_OPT_NAME,                                 // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPoolSizeMax"             ;
  const char* fieldName = "CACHEPOOLSIZEMAX"                        ;
  using dataType        = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHEPOOLSIZEMAX_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.poolsizemax,                                       // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPOOLSIZEMAX_ENV_NAME,                                 // envName
    CAOS_CACHEPOOLSIZEMAX_OPT_NAME,                                 // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPoolWait"                ;
  const char* fieldName = "CACHEPOOLWAIT"                           ;
  using dataType        = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHEPOOLWAIT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.poolwait,                                          // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPOOLWAIT_ENV_NAME,                                    // envName
    CAOS_CACHEPOOLWAIT_OPT_NAME,                                    // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPoolConnectionTimeout"   ;
  const char* fieldName = "CACHEPOOLCONNECTIONTIMEOUT"              ;
  using dataType        = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHEPOOLCONNECTIONTIMEOUT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.poolconnectiontimeout,                             // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPOOLCONNECTIONTIMEOUT_ENV_NAME,                       // envName
    CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME,                       // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPoolConnectionLifetime"  ;
  const char* fieldName = "CACHEPOOLCONNECTIONLIFETIME"             ;
  using dataType        = std::chrono::seconds                      ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHEPOOLCONNECTIONLIFETIME_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.poolconnectionlifetime,                            // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPOOLCONNECTIONLIFETIME_ENV_NAME,                      // envName
    CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME,                      // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Cache::Pool::setPoolConnectionIdletime"  ;
  const char* fieldName = "CACHEPOOLCONNECTIONIDLETIME"             ;
  using dataType        = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_CACHEPOOLCONNECTIONIDLETIME_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.poolconnectionidletime,                            // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CACHEPOOLCONNECTIONIDLETIME_ENV_NAME,                      // envName
    CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME,                      // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::setPoolConnectionIdletime()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Cache::Pool::init()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CAOS_USE_CACHE_REDIS
std::unique_ptr<IRepository> Cache::Pool::init(std::unique_ptr<IRepository>& database)
{
  return std::make_unique<Redis>(database, this->getConnectOpt(), this->getPoolOpt());
}
#endif
// -------------------------------------------------------------------------------------------------
// End of Cache::Pool::init()
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
