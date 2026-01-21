#include "PostgreSQL.hpp"

#ifdef CAOS_BUILD_EXAMPLES
#include "Query.hpp"
#endif

#if (defined(CAOS_STANDALONE_CODE)||defined(CAOS_TEMPLATE_CODE)||(defined CAOS_CROWCPP_CODE))
#include "../../src/include/Database/PostgreSQL/Query.hpp"
#endif

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
  const char* fName = "PostgreSQL::Pool::validateConnection";

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
        #if CAOS_VALIDATE_USING_TRANSACTION == 0
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
  const char* fName = "PostgreSQL::Pool::createConnection";

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
  const char* fName = "PostgreSQL::Pool::closeConnection";

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
  const char* fName = "PostgreSQL::Pool::closeConnection(Crowcpp endpoint)";

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
  const char* fName     = "Database::Pool::setKeepAlives"           ;
  const char* fieldName = "DBKEEPALIVES"                            ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBKEEPALIVES_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.keepalives,                                        // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBKEEPALIVES_ENV_NAME,                                     // envName
    CAOS_DBKEEPALIVES_OPT_NAME,                                     // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Database::Pool::setKeepAlivesIdle"       ;
  const char* fieldName = "DBKEEPALIVES_IDLE"                       ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBKEEPALIVES_IDLE_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.keepalives_idle,                                   // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBKEEPALIVES_IDLE_ENV_NAME,                                // envName
    CAOS_DBKEEPALIVES_IDLE_OPT_NAME,                                // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Database::Pool::setKeepAlivesInterval"   ;
  const char* fieldName = "DBKEEPALIVES_INTERVAL"                   ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBKEEPALIVES_INTERVAL_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.keepalives_interval,                               // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBKEEPALIVES_INTERVAL_ENV_NAME,                            // envName
    CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME,                            // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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
  const char* fName     = "Database::Pool::setKeepAlivesCount"      ;
  const char* fieldName = "DBKEEPALIVES_COUNT"                      ;
  using       dataType  = std::size_t                               ;

  Policy::NumberAtLeast<dataType> validator(
    fieldName,
    CAOS_DBKEEPALIVES_COUNT_LIMIT_MIN
  )                                                                 ;

  configureValue<dataType>(
    this->config.keepalives_count,                                  // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_DBKEEPALIVES_COUNT_ENV_NAME,                               // envName
    CAOS_DBKEEPALIVES_COUNT_OPT_NAME,                               // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
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





PostgreSQL::~PostgreSQL() {}
/***************************************************************************************************
 *
 *
 *
 *
 *
 **************************************************************************************************/
