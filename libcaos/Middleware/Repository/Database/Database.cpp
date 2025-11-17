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

#include "Database.hpp"
#include "Query.hpp"

#include <arpa/inet.h>

#ifdef CAOS_USE_DB_POSTGRESQL
#include "PostgreSQL/PostgreSQL.hpp"

#elif CAOS_USE_DB_MYSQL
#include "MySQL/MySQL.hpp"

#elif CAOS_USE_DB_MARIADB
#include "MariaDB/MariaDB.hpp"
#endif

constexpr const char* defaultFinal = "{} : Setting database {} to {} in {} environment";










/***************************************************************************************************
 *
 *
 * Database::Pool() Constructor/Destructor
 *
 *
 **************************************************************************************************/
Database::Pool::Pool()
  : connectionRefused(false),
    config({})
{
  if (this->terminalPtr==nullptr)
  {
    this->terminalPtr = &TerminalOptions::get_instance();
  }

  setUser()                 ;
  setPass()                 ;
  setHost()                 ;
  setPort()                 ;
  setName()                 ;

  setPoolSizeMin()          ;
  setPoolSizeMax()          ;
  if (this->getPoolSizeMin() > this->getPoolSizeMax())
  {
    throw std::out_of_range("DBPOOLSIZEMIN > DBPOOLSIZEMAX");
  }

  setPoolWait()             ;
  setPoolTimeout()          ;
  setConnectTimeout()       ;
  setMaxWait()              ;
  setHealthCheckInterval()  ;

#ifdef CAOS_USE_DB_POSTGRESQL
  setKeepAlives()           ;
  setKeepAlivesIdle()       ;
  setKeepAlivesInterval()   ;
  setKeepAlivesCount()      ;
  setConnectStr()           ;
#endif

#if (defined(CAOS_USE_DB_MYSQL)||defined(CAOS_USE_DB_MARIADB))
  setConnectOpt()           ;
#endif

  this->healthCheckThread_ = std::thread([this]() {
    this->healthCheckLoop();
  });
}





Database::Pool::~Pool()
{
  this->printConnectionStats();

  running.store(false, std::memory_order_release);

  this->condition.notify_all();
  this->shutdown_cv_.notify_all();

  if (this->healthCheckThread_.joinable())
  {
    this->healthCheckThread_.join();
  }
}
/***************************************************************************************************
 *
 *
 *
 *
 *
 **************************************************************************************************/










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setUser()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setUser()
{
  const char* fName     = "Database::Pool::setUser"                 ;
  const char* fieldName = "DBUSER"                                  ;
  using       dataType  = std::string                               ;

  Policy::StringIsPresent validator(fieldName)                      ;

  configureValue<dataType>(
    this->config.user,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBUSER_ENV_NAME,                                           // envName
    CAOS_DBUSER_OPT_NAME,                                           // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setUser()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setPass()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setPass()
{
  const char* fName     = "Database::Pool::setPass"                 ;
  const char* fieldName = "DBPASS"                                  ;
  using       dataType  = std::string                               ;

  Policy::StringIsPresent validator(fieldName)                      ;

  configureValue<dataType>(
    this->config.pass,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBPASS_ENV_NAME,                                           // envName
    CAOS_DBPASS_OPT_NAME,                                           // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setPass()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setHost()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setHost()
{
  const char* fName     = "Database::Pool::setHost"                 ;
  const char* fieldName = "DBHOST"                                  ;
  using       dataType  = std::string                               ;

  Policy::HostValidator validator(fieldName)                        ;

  configureValue<dataType>(
    this->config.host,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBHOST_ENV_NAME,                                           // envName
    CAOS_DBHOST_OPT_NAME,                                           // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setHost()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setPort()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setPort()
{
  const char* fName     = "Database::Pool::setPort"                 ;
  const char* fieldName = "DBPORT"                                  ;
  using       dataType  = std::uint16_t                             ;

  Policy::PortValidator validator("DBPORT")                         ;

  configureValue<dataType>(
    this->config.port,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBPORT_ENV_NAME,                                           // envName
    CAOS_DBPORT_OPT_NAME,                                           // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setPort()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setName()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setName()
{
  const char* fName     = "Database::Pool::setName"                 ;
  const char* fieldName = "DBNAME"                                  ;
  using       dataType  = std::string                               ;

  Policy::StringIsPresent validator(fieldName)                      ;

  configureValue<dataType>(
    this->config.name,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBNAME_ENV_NAME,                                           // envName
    CAOS_DBNAME_OPT_NAME,                                           // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setName()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setPoolSizeMin()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setPoolSizeMin()
{
  const char* fName     = "Database::Pool::setPoolSizeMin"          ;
  const char* fieldName = "DBPOOLSIZEMIN"                           ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBPOOLSIZEMIN_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.poolsizemin,                                       // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBPOOLSIZEMIN_ENV_NAME,                                    // envName
    CAOS_DBPOOLSIZEMIN_OPT_NAME,                                    // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setPoolSizeMin()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setPoolSizeMax()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setPoolSizeMax()
{
  const char* fName     = "Database::Pool::setPoolSizeMax"          ;
  const char* fieldName = "DBPOOLSIZEMAX"                           ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBPOOLSIZEMAX_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.poolsizemax,                                       // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBPOOLSIZEMAX_ENV_NAME,                                    // envName
    CAOS_DBPOOLSIZEMAX_OPT_NAME,                                    // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setPoolSizeMax()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setPoolWait()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setPoolWait()
{
  const char* fName     = "Database::Pool::setPoolWait"             ;
  const char* fieldName = "DBPOOLWAIT"                              ;
  using       dataType  = std::uint32_t                             ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBPOOLWAIT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.poolwait,                                          // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBPOOLWAIT_ENV_NAME,                                       // envName
    CAOS_DBPOOLWAIT_OPT_NAME,                                       // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setPoolWait()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setPoolTimeout()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setPoolTimeout()
{
  const char* fName     = "Database::Pool::setPoolTimeout"          ;
  const char* fieldName = "DBPOOLTIMEOUT"                           ;
  using       dataType  = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBPOOLTIMEOUT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.pooltimeout,                                       // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBPOOLTIMEOUT_ENV_NAME,                                    // envName
    CAOS_DBPOOLTIMEOUT_OPT_NAME,                                    // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setPoolTimeout()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setConnectTimeout()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setConnectTimeout()
{
  const char* fName     = "Database::Pool::setConnectTimeout"       ;
  const char* fieldName = "DBCONNECT_TIMEOUT"                       ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBCONNECT_TIMEOUT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.connect_timeout,                                   // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBCONNECT_TIMEOUT_ENV_NAME,                                // envName
    CAOS_DBCONNECT_TIMEOUT_OPT_NAME,                                // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setConnectTimeout()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setMaxWait()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setMaxWait()
{
  const char* fName     = "Database::Pool::setMaxWait"              ;
  const char* fieldName = "DBMAXWAIT"                               ;
  using       dataType  = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBMAXWAIT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.maxwait,                                           // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBMAXWAIT_ENV_NAME,                                        // envName
    CAOS_DBMAXWAIT_OPT_NAME,                                        // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setMaxWait()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setHealthCheckInterval()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setHealthCheckInterval()
{
  const char* fName     = "Database::Pool::setHealthCheckInterval"  ;
  const char* fieldName = "DBHEALTHCHECKINTERVAL"                   ;
  using       dataType  = std::chrono::milliseconds                 ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBHEALTHCHECKINTERVAL_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType, std::uint32_t>(
    this->config.healthCheckInterval,                               // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBHEALTHCHECKINTERVAL_ENV_NAME,                            // envName
    CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME,                            // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setHealthCheckInterval()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










const std::string&                Database::Pool::getUser()                 const noexcept { return this->config.user;                  }
const std::string&                Database::Pool::getPass()                 const noexcept { return this->config.pass;                  }
const std::string&                Database::Pool::getHost()                 const noexcept { return this->config.host;                  }
const std::uint16_t&              Database::Pool::getPort()                 const noexcept { return this->config.port;                  }
const std::string&                Database::Pool::getName()                 const noexcept { return this->config.name;                  }
const std::size_t&                Database::Pool::getPoolSizeMin()          const noexcept { return this->config.poolsizemin;           }
const std::size_t&                Database::Pool::getPoolSizeMax()          const noexcept { return this->config.poolsizemax;           }
const std::uint32_t&              Database::Pool::getPoolWait()             const noexcept { return this->config.poolwait;              }
const std::chrono::milliseconds&  Database::Pool::getPoolTimeout()          const noexcept { return this->config.pooltimeout;           }
const std::size_t&                Database::Pool::getConnectTimeout()       const noexcept { return this->config.connect_timeout;       }

#ifdef CAOS_USE_DB_POSTGRESQL
const std::size_t&                Database::Pool::getKeepAlives()           const noexcept { return this->config.keepalives;            }
const std::size_t&                Database::Pool::getKeepAlivesIdle()       const noexcept { return this->config.keepalives_idle;       }
const std::size_t&                Database::Pool::getKeepAlivesInterval()   const noexcept { return this->config.keepalives_interval;   }
const std::size_t&                Database::Pool::getKeepAlivesCount()      const noexcept { return this->config.keepalives_count;      }
const std::string&                Database::Pool::getConnectStr()           const noexcept { return this->config.connection_string;     }
#elif (defined(CAOS_USE_DB_MYSQL)||defined(CAOS_USE_DB_MARIADB))
      sql::ConnectOptionsMap&     Database::Pool::getConnectOpt()                 noexcept { return this->config.connection_options;    }
#endif

const std::chrono::milliseconds&  Database::Pool::getMaxWait()              const noexcept { return this->config.maxwait;               }
const std::chrono::milliseconds&  Database::Pool::getHealthCheckInterval()  const noexcept { return this->config.healthCheckInterval;   }







// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::get_metrics()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Database::Pool::Metrics Database::Pool::get_metrics() const noexcept
// {
//   std::lock_guard<std::mutex> lock(Database::Pool::available_mutex_);

//   return Metrics
//   {
//     Database::Pool::available_.size(),
//     total_known_.load(std::memory_order_relaxed),
//     creations_.load(std::memory_order_relaxed),
//     failures_.load(std::memory_order_relaxed),
//   };
// }
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::get_metrics()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::getAvailableConnections()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::size_t Database::Pool::getAvailableConnections() noexcept
{
  std::size_t count = 0;

  Database::Pool::PoolData& pool = this->getPoolData();

  std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

  // Cerca la unique_ptr corrispondente nella mappa
  for (auto it = pool.connections.begin(); it != pool.connections.end(); ++it)
  {
    auto& metrics = it->second;

    if (!metrics.is_acquired)
    {
      count++;
    }
  }

  return count;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::getAvailableConnections()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::getTotalConnections()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::size_t Database::Pool::getTotalConnections() noexcept
{
  Database::Pool::PoolData& pool = this->getPoolData();

  std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

  return pool.connections.size();
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::getTotalConnections()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::checkPoolSize()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool Database::Pool::checkPoolSize(std::size_t& askingPoolSize) noexcept
{
  static constexpr const char* fName = "Database::Pool::checkPoolSize";

  const std::size_t totalConnections  = this->getTotalConnections();
  const std::size_t poolSizeMax       = this->getPoolSizeMax();

  if (totalConnections >= poolSizeMax)  // Don't saturate Database connections
  {
    spdlog::warn("[{}] Connection pool limit reached, verify CAOS_DBPOOLSIZEMAX variable, current poolsize is {}, asked for {} new connections, max poolsize is {}",
                 fName,
                 totalConnections,
                 askingPoolSize,
                 poolSizeMax);

    return false;
  }

  return true;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::checkPoolSize()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::init()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::size_t Database::Pool::init(std::size_t count)
{
  static constexpr const char* fName = "Database::Pool::init";

  std::size_t i = 0;
  std::size_t pool_size = (count>0) ? count : this->getPoolSizeMin();

  spdlog::info("[{}] New pool building", fName);

  while (i < pool_size                                                                              // Create connections until the requested size is reached
         && running.load(std::memory_order_acquire)                                                // Stop if a signal detected
         && !this->connectionRefused.load(std::memory_order_acquire))                               // Stop if a previous connection was refused
  {
    try
    {
      this->createConnection(pool_size);
      spdlog::trace("[{}] New Database connection established", fName);
    }
    catch (const repository::broken_connection& e)
    {
      throw;
    }

    i++;
  }

  return i;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::init()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::getPoolData()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Database::Pool::PoolData& Database::Pool::getPoolData() noexcept
{
  static PoolData instance(this->getPoolSizeMax());
  return instance;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::getPoolData()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------












// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::healthCheckLoop()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::healthCheckLoop()
{
  static constexpr const char* fName = "Database::Pool::healthCheckLoop";

  while (running.load(std::memory_order_acquire))
  {
    spdlog::trace("Running {}", fName);

    {
      Database::Pool::PoolData& pool = this->getPoolData();

      std::unique_lock<std::mutex> lock(pool.connections_mutex);

      try
      {
        if (!pool.connections.empty())
        {
          spdlog::debug("Performing health check on existing connections");

          for (auto it = pool.connections.begin(); it != pool.connections.end(); ++it)
          {
            auto& [connection, metrics] = *it;

            if (metrics.is_acquired)
            {
              continue;
            }

            try
            {
              metrics.is_acquired = true;                                                         // Don't let threads acquire this connection while validating

              this->validateConnection(connection);

              metrics.is_acquired = false;

              spdlog::trace("[{}] Connection is valid", fName);
            }
            catch(...)
            {
              spdlog::info("[{}] Invalid connection marked for removal", fName);
              spdlog::trace("[{}] Connection invalid", fName);
              pool.connectionsToRemove.push_back(it);
            }
          }
        }

        this->cleanupMarkedConnections();                                                           // Remove invalid connections

        std::size_t totalConnections = pool.connections.size();

        int connectionDiff = static_cast<int>(totalConnections - this->getPoolSizeMin());

        if (connectionDiff < 0)
        {
          this->init(static_cast<std::size_t>(-connectionDiff));                                    // Refill pool
        }
      }
      catch (const repository::broken_connection& e)
      {
        spdlog::error("[{}] Database unreachable or port closed", fName);
      }
    }

    condition.notify_all();

    {
      std::unique_lock<std::mutex> waitlock(this->shutdown_mutex_);
      shutdown_cv_.wait_for(
            waitlock,
            this->getHealthCheckInterval(),
            [this]{
                this->connectionRefused = false;
                return !running.load(std::memory_order_acquire);
            }
      );
    }
  }
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::healthCheckLoop()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::calculateAverageDuration()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::chrono::milliseconds Database::Pool::calculateAverageDuration()
{
  std::chrono::milliseconds total_duration{0};
  int total_operations = 0;

  {
    Database::Pool::PoolData& pool = this->getPoolData();

    std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

    if (pool.connections.empty())
    {
      return std::chrono::milliseconds(0);
    }

    for (const auto& [connection, metrics] : pool.connections)
    {
      total_duration += metrics.total_duration;
      total_operations += metrics.usage_count;
    }
  }

  if (total_operations == 0)
  {
    return std::chrono::milliseconds(0);
  }

  return std::chrono::milliseconds(total_duration / total_operations);
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::calculateAverageDuration()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::acquireConnection()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INIT_CONNECTION()                         \
  try                                             \
  {                                               \
    this->init(1);                                \
  }                                               \
  catch (const repository::broken_connection& e)  \
  {                                               \
    throw;                                        \
  }

dboptuniqptr Database::Pool::acquireConnection()
{
  static constexpr const char* fName = "Database::Pool::acquireConnection";

  if (!running.load(std::memory_order_acquire))
  {
    return std::nullopt;
  }

  {
    Database::Pool::PoolData& pool = this->getPoolData();

    std::unique_lock<std::mutex> lock(pool.connections_mutex);

    if (pool.connections.empty())
    {
      // Pool is empty! Do not await healthCheckLoop(). Create emergency connection
      INIT_CONNECTION()
    }
    else
    {
      for (auto it = pool.connections.begin(); it != pool.connections.end(); ++it)
      {
        const auto& connection  = it->first                                                   ;
        auto&       metrics     = it->second                                                  ;

        if (!metrics.is_acquired)
        {
          try
          {
            #if CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE==1
            if (this->validateConnection(connection))
            {
            #else
            if (connection->is_open())
            {
            #endif
              auto now = std::chrono::steady_clock::now()                                     ;
              metrics.start_time    = now                                                     ;
              metrics.last_acquired = now                                                     ;
              metrics.is_acquired   = true                                                    ;
              metrics.usage_count++                                                           ;

              return &connection                                                              ;
            }

            throw repository::broken_connection("Connection lost");

          }
          catch (const repository::broken_connection& e)
          {
            // Remove connection
            pool.connectionsToRemove.push_back(it);
            this->cleanupMarkedConnections();

            throw;
          }
        }
      }

      // Pool count is not enough! Try to create an emergency connection
      INIT_CONNECTION()
    }
  }

  static std::atomic<std::size_t> limiter                                     {0}         ;

  std::size_t current = limiter.fetch_add(1, std::memory_order_acq_rel) + 1               ;

  if (current >= CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED)
  {
    std::size_t expected = current;

    if (limiter.compare_exchange_strong(expected, 0,
                                        std::memory_order_release,
                                        std::memory_order_relaxed))
    {
      spdlog::warn("[{}] : Unable to acquire Database connection - {} failed requests (pool busy)", fName, current);
      spdlog::info("[{}] : Database connection limit exceeded: reduce thread pool size or increase max_connections parameter", fName);
    }
  }

  return std::nullopt;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::acquireConnection()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::handleInvalidConnection()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::handleInvalidConnection()
{
  std::size_t newConnection = this->init(1);

  if (newConnection==1)
  {
    this->condition.notify_all();
  }

}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::handleInvalidConnection()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::cleanupMarkedConnections()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::cleanupMarkedConnections()
{
  static constexpr const char* fName = "Database::Pool::cleanupMarkedConnections";

  Database::Pool::PoolData& pool = this->getPoolData();

  std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

  for (auto it : pool.connectionsToRemove)
  {
    try
    {
      if (it != pool.connections.end())
      {
        auto& [connection, metrics] = *it;
        connection->close();
        spdlog::debug("[{}] Removed connection (used {} times)", fName, metrics.usage_count);
        pool.connections.erase(it);
      }
    }
    catch (const std::exception& e)
    {
      spdlog::error("[{}] Error removing connection: {}", fName, e.what());
    }
  }

  pool.connectionsToRemove.clear();
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::cleanupMarkedConnections()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::cleanupIdleConnections()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// void Database::Pool::cleanupIdleConnections()
// {
//   std::lock_guard<std::mutex> lock(Database::Pool::connections_mutex);
//   auto now = std::chrono::steady_clock::now();

//   for (auto it = Database::Pool::connections.begin(); it != Database::Pool::connections.end(); )
//   {
//     if (Database::Pool::connections.size()<=Database::Pool::getPoolSizeMin())
//     {
//       break;
//     }
//     const auto& [connection, metrics] = *it;

//     // Convert weak_ptr into a shared_ptr

//     // auto idle_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - metrics.end_time);

//     // std::cout << (metrics.is_acquired.load(std::memory_order_acquire) ? "ACQUIRED" : "IDLE")
//     //           << " - Idle time: " << idle_time.count() << "ms"
//     //           << " - Timeout: " << Database::Pool::getPoolTimeout().count() << "ms\n";

//     // Se la connessione non è in uso ed è inattiva da troppo tempo
//     if (!metrics.is_acquired && ((now - metrics.end_time) > Database::Pool::getPoolTimeout()))
//     {
//       // Log delle metriche prima di rimuovere
//       auto total_usage_ms = std::chrono::duration_cast<std::chrono::milliseconds>(metrics.total_duration);
//       std::cout << "Closing connection - Total usage: "
//                 << total_usage_ms.count() << "ms, "
//                 << "Used " << metrics.usage_count << " times\n";

//       // USA LO shared_ptr OTTENUTO DAL LOCK
//       Database::Pool::closeConnection(connection);
//     }
//     else
//     {
//       ++it;
//     }
//   }
// }
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::cleanupIdleConnections()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::acquire()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::optional<Database::ConnectionWrapper> Database::Pool::acquire()
{
  try
  {
    auto connection_opt = this->acquireConnection();

    if (connection_opt) {
        return ConnectionWrapper(
            connection_opt,
            [this](dboptuniqptr conn_ptr){
              this->releaseConnection(conn_ptr);
            });
    }
  }
  catch (const repository::broken_connection& e)
  {
    throw;
  }

  return std::nullopt;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::acquire()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::releaseConnection()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::releaseConnection(dboptuniqptr connection_opt)
{
  if (connection_opt.has_value())
  {
    const dbuniq* unique_ptr_ptr = connection_opt.value();

    Database::Pool::PoolData& pool = this->getPoolData();

    std::unique_lock lock(pool.connections_mutex, std::try_to_lock);

    for (auto it = pool.connections.begin(); it != pool.connections.end(); ++it)
    {
      if (&it->first == unique_ptr_ptr)
      {
        auto& metrics = it->second;
        auto now = std::chrono::steady_clock::now();
        metrics.end_time = now;
        metrics.last_duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - metrics.start_time);
        metrics.total_duration += metrics.last_duration;
        metrics.is_acquired = false;

        Database::Pool::condition.notify_one();

        return;
      }
    }

    spdlog::warn("Connection not found in pool during release");
  }
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::releaseConnection()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::getTotalDuration()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const std::chrono::milliseconds Database::Pool::getTotalDuration(const dbuniq& connection)
{
  Database::Pool::PoolData& pool = this->getPoolData();

  std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

  if (auto it = pool.connections.find(connection); it != pool.connections.end())
  {
    return it->second.total_duration;
  }

  return std::chrono::milliseconds(0);
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::getTotalDuration()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::getLastDuration()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const std::chrono::milliseconds Database::Pool::getLastDuration(const dbuniq& connection)
{
  Database::Pool::PoolData& pool = this->getPoolData();

  std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

  if (auto it = pool.connections.find(connection); it != pool.connections.end())
  {
    return it->second.last_duration;
  }

  return std::chrono::milliseconds(0);
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::getLastDuration()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::getUsageCount()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int Database::Pool::getUsageCount(const dbuniq& connection) noexcept
{
  Database::Pool::PoolData& pool = this->getPoolData();

  std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

  if (auto it = pool.connections.find(connection); it != pool.connections.end())
  {
    return it->second.usage_count;
  }

  return 0;
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::getUsageCount()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::printConnectionStats()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::printConnectionStats()
{
  std::chrono::milliseconds total_duration{0};
  std::size_t total_uses = 0;
  std::size_t active_connections = 0;
  std::size_t idle_connections = 0;
  std::size_t size = 0;

  {
    Database::Pool::PoolData& pool = this->getPoolData();

    std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

    size = pool.connections.size();

    for (const auto& [conn, metrics] : pool.connections)
    {
      total_duration += metrics.total_duration;
      total_uses += metrics.usage_count;

      if (metrics.is_acquired)
      {
        active_connections++;
      }
      else
      {
        idle_connections++;
      }
    }
  }

  std::cout << "=== Connection Pool Statistics ===\n"
            << "Total connections: "                  << size << "\n"
            << "Active: "                             << active_connections         << "\n"
            << "Idle: "                               << idle_connections           << "\n"
            << "Total usage time: "                   << std::chrono::duration_cast<std::chrono::milliseconds>(total_duration).count() << "ms\n"
            << "Total operations: "                   << total_uses                 << "\n";

  if (total_uses > 0)
  {
    auto avg_duration = total_duration / total_uses;
    std::cout << "Average operation time: "
              << avg_duration.count() << "ms\n";
  }
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::printConnectionStats()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

Database::Database()
  : type(
    #ifdef CAOS_USE_DB_POSTGRESQL
    DatabaseType::PostgreSQL
    #elif defined(CAOS_USE_DB_MYSQL)
    DatabaseType::MySQL
    #elif defined(CAOS_USE_DB_MARIADB)
    DatabaseType::MariaDB
    #else
    #error "Unknown DatabaseType"
    #endif
  )
{
  spdlog::trace("Database run init...");

  // Setup Database
  #ifdef CAOS_USE_DB_POSTGRESQL
  spdlog::trace("DatabaseType is PostgreSQL");
  this->database  = std::make_unique<PostgreSQL>(this);

  #elif defined(CAOS_USE_DB_MYSQL)
  spdlog::trace("DatabaseType is MySQL");
  this->database  = std::make_unique<MySQL>(this);

  #elif defined(CAOS_USE_DB_MARIADB)
  spdlog::trace("DatabaseType is MariaDB");
  this->database  = std::make_unique<MariaDB>(this);

  #else
  #error "Unknown DatabaseType"
  #endif

  // Run Pool
  this->pool = std::make_unique<Pool>();

  spdlog::info("Database init ok");
}

Database::~Database()
{
  spdlog::trace("Destroying Database");

  this->pool.reset();
  this->database.reset();

  spdlog::info("Database destroyed");
}

std::optional<Database::ConnectionWrapper>  Database::acquire()                                 { return this->pool->acquire();               }
void                                        Database::releaseConnection(dboptuniqptr connection){ this->pool->releaseConnection(connection);  }
