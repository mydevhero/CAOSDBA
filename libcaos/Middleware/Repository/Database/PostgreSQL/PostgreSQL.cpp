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

#include "PostgreSQL.hpp"
#include "Query.hpp"

constexpr const char* defaultFinal = "{} : Setting database {} to {} in {}  environment";

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of PostgreSQL::Pool::setConnectStr()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void   Database::Pool::setConnectStr() noexcept
{
  std::ostringstream oss;

  oss << "host="                  << this->getHost()
      << " port="                 << this->getPort()
      << " dbname="               << this->getName()
      << " user="                 << this->getUser()
      << " password="             << this->getPass()
      << " connect_timeout="      << this->getConnectTimeout()
      << " keepalives="           << this->getKeepAlives()
      << " keepalives_interval="  << this->getKeepAlivesInterval()
      << " keepalives_idle="      << this->getKeepAlivesIdle()
      << " keepalives_count="     << this->getKeepAlivesCount();

  this->config.connection_string = oss.str();
}
// -------------------------------------------------------------------------------------------------
// End of PostgreSQL::Pool::setConnectStr()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of PostgreSQL::Pool::validateConnection()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool Database::Pool::validateConnection(const dbuniq& connection)
{
  static constexpr const char* fName = "PostgreSQL::Pool::validateConnection";

  if (!connection)
  {
    return false;
  }

  try
  {
    if (connection->is_open())
    {
      spdlog::trace("[{}] Health check query", fName);

      pqxx::result result;

      {
        #if VALIDATE_USING_TRANSACTION == 0
        pqxx::nontransaction nontx(*connection);
        result = nontx.exec("SELECT 1");
        #else
        pqxx::work tx(*connection);
        result=tx.exec("SELECT 1");
        tx.commit();
        #endif
      }

      if (!result.empty())
      {
        return true;
      }
    }
  }
  catch (const pqxx::broken_connection& e)
  {
    spdlog::error("[{}] :B {}", fName, e.what());
    throw repository::broken_connection("Connection invalid");
  }
  catch (const std::exception& e)
  {
    spdlog::error("[{}] :A {}", fName, e.what());
  }
  catch (...)
  {
    spdlog::error("[{}] : unknown exception", fName);
  }

  return false;
}
// -------------------------------------------------------------------------------------------------
// End of PostgreSQL::Pool::validateConnection()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of PostgreSQL::Pool::createConnection()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::createConnection(std::size_t& pool_size)
{
  static constexpr const char* fName = "PostgreSQL::Pool::createConnection";

  static std::atomic<bool> loggedOnce {false};

  try
  {
    spdlog::debug("[{}] Creating new connection", fName);

    if(!this->checkPoolSize(pool_size))                                                 // Don't saturate PostgreSQL connections
    {
      return;
    }

    Database::Pool::PoolData& pool = this->getPoolData();

    auto connection = std::make_unique<dbconn>(this->getConnectStr());

    if (this->validateConnection(connection))
    {
      ConnectionMetrics metrics;

      pool.connections.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(std::move(connection)),
        std::forward_as_tuple(std::move(metrics))
      );

      spdlog::info("[{}] New valid connection created (total: {})",
                  fName, this->getTotalConnections());

      this->connectionRefused = false;

      loggedOnce.store(false, std::memory_order_release);

      return;
    }

    connection->close();
  }
  catch (const pqxx::broken_connection& e)
  {
    throw repository::broken_connection("Server unreachable or port closed");
  }
  catch (const std::exception& e)
  {
    if (!loggedOnce.load(std::memory_order_acquire))
    {
      spdlog::error("[{}] Exception during connection creation: {}", fName, e.what());
      loggedOnce.store(true, std::memory_order_release);
    }
  }
  catch (...)
  {
    if (!loggedOnce.load(std::memory_order_acquire))
    {
      spdlog::error("[{}] Unknown exception during connection creation", fName);
      loggedOnce.store(true, std::memory_order_release);
    }
  }

  return;
}
// -------------------------------------------------------------------------------------------------
// End of PostgreSQL::Pool::createConnection()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of PostgreSQL::Pool::closeConnection()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Internal use
void Database::Pool::closeConnection(const dbuniq& connection)
{
  static constexpr const char* fName = "PostgreSQL::Pool::closeConnection(1)";

  try
  {
    if (!connection)
    {
      spdlog::warn("[{}] Attempted to close null connection", fName);
      return;
    }

    {
      Database::Pool::PoolData& pool = this->getPoolData();

      std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

      if (auto it = pool.connections.find(connection); it != pool.connections.end())
      {
        connection->close();
        spdlog::debug("[{}] Removed metrics for connection (used {} times)", fName, it->second.usage_count);
        it = pool.connections.erase(it);

        return;
      }
    }
  }
  catch (const pqxx::broken_connection& e)
  {
    spdlog::debug("[{}] Connection already broken: {}", fName, e.what());
  }
  catch (const pqxx::sql_error& e)
  {
    spdlog::warn("[{}] SQL error during connection close: {}", fName, e.what());
  }
  catch (const std::exception& e)
  {
    spdlog::error("[{}] Exception while closing connection: {}", fName, e.what());
  }
  catch (...)
  {
    spdlog::critical("[{}] Unknown exception while closing connection", fName);
  }
}


// Crow's endpoint use
void Database::Pool::closeConnection(std::optional<Database::ConnectionWrapper>& connection)
{
  static constexpr const char* fName = "PostgreSQL::Pool::closeConnection(2)";

  try
  {
    if (!connection)
    {
      spdlog::warn("[{}] Attempted to close null connection", fName);
      return;
    }

    {
      Database::Pool::PoolData& pool = this->getPoolData();

      std::unique_lock<std::mutex> lock(pool.connections_mutex, std::try_to_lock);

      auto* raw = connection.value().get ();
      auto it = std::find_if (pool.connections.begin(), pool.connections.end(), [raw](auto const& pair) {
        return pair.first.get() == raw;
      });

      if (it != pool.connections.end())
      {
        raw->close();
        spdlog::debug("[{}] Removed metrics for connection (used {} times)", fName, it->second.usage_count);
        it = pool.connections.erase(it);

        return;
      }
    }
  }
  catch (const pqxx::broken_connection& e)
  {
    spdlog::debug("[{}] Connection already broken: {}", fName, e.what());
  }
  catch (const pqxx::sql_error& e)
  {
    spdlog::warn("[{}] SQL error during connection close: {}", fName, e.what());
  }
  catch (const std::exception& e)
  {
    spdlog::error("[{}] Exception while closing connection: {}", fName, e.what());
  }
  catch (...)
  {
    spdlog::critical("[{}] Unknown exception while closing connection", fName);
  }
}
// -------------------------------------------------------------------------------------------------
// End of PostgreSQL::Pool::closeConnection()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setKeepAlives()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setKeepAlives()
{
  static constexpr const char* fName = "Database::Pool::setKeepAlives";

  try
  {
    // Set primary database keepalives for production environment ----------------------------------
    if (this->terminalPtr->has(CAOS_OPT_DBKEEPALIVES_NAME))
    {
      this->config.keepalives = this->terminalPtr->get<std::size_t>(CAOS_OPT_DBKEEPALIVES_NAME);
    }
    else if (const char* keepalives = std::getenv(CAOS_ENV_DBKEEPALIVES_NAME))
    {
      this->config.keepalives = static_cast<std::size_t>(std::stoi(keepalives));
    }
    else
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES) && (CAOS_DEFAULT_DBKEEPALIVES + 0) > 0
      this->config.keepalives = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative database keepalives for test or dev environment -----------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_ON_DEV_OR_TEST) && (CAOS_DEFAULT_DBKEEPALIVES_ON_DEV_OR_TEST + 0) > 0
      this->config.keepalives = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_ON_DEV_OR_TEST);
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
               "keepalives",
               this->config.keepalives,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setKeepAlives()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setKeepAlivesIdle()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setKeepAlivesIdle()
{
  static constexpr const char* fName = "Database::Pool::setKeepAlivesIdle";

  try
  {
    // Set primary database keepalives_idle for production environment -----------------------------
    if (this->terminalPtr->has(CAOS_OPT_DBKEEPALIVES_IDLE_NAME))
    {
      this->config.keepalives_idle = this->terminalPtr->get<std::size_t>(CAOS_OPT_DBKEEPALIVES_IDLE_NAME);
    }
    else if (const char* keepalives_idle = std::getenv(CAOS_ENV_DBKEEPALIVES_IDLE_NAME))
    {
      this->config.keepalives_idle = static_cast<std::size_t>(std::stoi(keepalives_idle));
    }
    else
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_IDLE) && (CAOS_DEFAULT_DBKEEPALIVES_IDLE + 0) > 0
      this->config.keepalives_idle = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_IDLE);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative database keepalives_idle for test or dev environment ------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_IDLE_ON_DEV_OR_TEST) && (CAOS_DEFAULT_DBKEEPALIVES_IDLE_ON_DEV_OR_TEST + 0) > 0
      this->config.keepalives_idle = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_IDLE_ON_DEV_OR_TEST);
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
               "keepalives_idle",
               this->config.keepalives_idle,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setKeepAlivesIdle()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::Pool::setKeepAlivesInterval()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setKeepAlivesInterval()
{
  static constexpr const char* fName = "Database::Pool::setKeepAlivesInterval";

  try
  {
    // Set primary database keepalives_interval for production environment -------------------------
    if (this->terminalPtr->has(CAOS_OPT_DBKEEPALIVES_INTERVAL_NAME))
    {
      this->config.keepalives_interval = this->terminalPtr->get<std::size_t>(CAOS_OPT_DBKEEPALIVES_INTERVAL_NAME);
    }
    else if (const char* keepalives_interval = std::getenv(CAOS_ENV_DBKEEPALIVES_INTERVAL_NAME))
    {
      this->config.keepalives_interval = static_cast<std::size_t>(std::stoi(keepalives_interval));
    }
    else
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_INTERVAL) && (CAOS_DEFAULT_DBKEEPALIVES_INTERVAL + 0) > 0
      this->config.keepalives_interval = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_INTERVAL);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative database keepalives_interval for test or dev environment --------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_INTERVAL_ON_DEV_OR_TEST) && (CAOS_DEFAULT_DBKEEPALIVES_INTERVAL_ON_DEV_OR_TEST + 0) > 0
      this->config.keepalives_interval = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_INTERVAL_ON_DEV_OR_TEST);
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
               "keepalives_interval",
               this->config.keepalives_interval,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setKeepAlivesInterval()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of Database::setKeepAlivesCount()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::Pool::setKeepAlivesCount()
{
  static constexpr const char* fName = "Database::Pool::setKeepAlivesCount";

  try
  {
    // Set primary database keepalives_count for production environment ----------------------------
    if (this->terminalPtr->has(CAOS_OPT_DBKEEPALIVES_COUNT_NAME))
    {
      this->config.keepalives_count = this->terminalPtr->get<std::size_t>(CAOS_OPT_DBKEEPALIVES_COUNT_NAME);
    }
    else if (const char* keepalives_count = std::getenv(CAOS_ENV_DBKEEPALIVES_COUNT_NAME))
    {
      this->config.keepalives_count = static_cast<std::size_t>(std::stoi(keepalives_count));
    }
    else
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_COUNT) && (CAOS_DEFAULT_DBKEEPALIVES_COUNT + 0) > 0
      this->config.keepalives_count = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_COUNT);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative database keepalives_count for test or dev environment -----------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_DBKEEPALIVES_COUNT_ON_DEV_OR_TEST) && (CAOS_DEFAULT_DBKEEPALIVES_COUNT_ON_DEV_OR_TEST + 0) > 0
      this->config.keepalives_count = static_cast<std::size_t>(CAOS_DEFAULT_DBKEEPALIVES_COUNT_ON_DEV_OR_TEST);
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
               "keepalives_count",
               this->config.keepalives_count,
               this->environmentRef->getName());
}
// -------------------------------------------------------------------------------------------------
// End of Database::Pool::setKeepAlivesCount()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
















/***************************************************************************************************
 *
 *
 * PostgreSQL() Constructor/Destructor
 *
 *
 **************************************************************************************************/
PostgreSQL::PostgreSQL(Database* database_):database(database_)
{
  spdlog::trace("PostgreSQL init");
}





PostgreSQL::~PostgreSQL()
{
  running_.store(false, std::memory_order_release);
}
/***************************************************************************************************
 *
 *
 *
 *
 *
 **************************************************************************************************/
