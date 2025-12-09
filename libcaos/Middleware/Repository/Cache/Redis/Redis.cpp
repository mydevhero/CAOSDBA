#include "Redis.hpp"

#ifdef CAOS_BUILD_EXAMPLES
#include "Query.hpp"
#endif

#if (defined(CAOS_STANDALONE_CODE)||defined(CAOS_TEMPLATE_CODE)||(defined CAOS_CROWCPP_CODE))
#include "../../src/include/Cache/Redis/Query.hpp"
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
