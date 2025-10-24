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

#include <terminal_options.hpp>

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Init of implementation of TerminalOptions
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
std::unique_ptr<TerminalOptions> TerminalOptions::instance_ = nullptr;

TerminalOptions::TerminalOptions(int argc, char** argv) : argc(argc), argv(argv)
{
  options = std::make_unique<cxxopts::Options>(argv[0], CAOS_OFFICIALNAME);

  options->add_options()
    ("help"                                           , "Show help"                                                                                                                                       )
    ("version"                                        , "Get build version&timestamp"                                                                                                                     )

    // Log
    (CAOS_LOG_SEVERITY_OPT_NAME                       , "Log Terminal Severity"           , cxxopts::value<std::string>()->default_value(CAOS_LOG_SEVERITY)                                )

    // CrowCpp
#ifdef CAOS_USE_CROWCPP
    (CAOS_CROWCPP_HOST_OPT_NAME                       , "Bind to address"                 , cxxopts::value<std::string>()->default_value(CAOS_CROWCPP_HOST)                                      )
    (CAOS_CROWCPP_PORT_OPT_NAME                       , "Bind to port"                    , cxxopts::value<std::uint16_t>()->default_value(std::to_string(CAOS_CROWCPP_PORT))               )
    (CAOS_CROWCPP_THREADS_OPT_NAME                    , "Number of threads"               , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_CROWCPP_THREADS))                    )
#endif

    // Cache
#ifdef CAOS_USE_CACHE
    (CAOS_CACHEUSER_OPT_NAME                          , "Cache User"                      , cxxopts::value<std::string>()->default_value(CAOS_CACHEUSER)                                          )
    (CAOS_CACHEPASS_OPT_NAME                          , "Cache Password"                  , cxxopts::value<std::string>()->default_value(CAOS_CACHEPASS)                                          )
    (CAOS_CACHEHOST_OPT_NAME                          , "Cache Host"                      , cxxopts::value<std::string>()->default_value(CAOS_CACHEHOST)                                          )
    (CAOS_CACHEPORT_OPT_NAME                          , "Cache Port"                      , cxxopts::value<std::uint16_t>()->default_value(std::to_string(CAOS_CACHEPORT))                        )
    (CAOS_CACHECLIENTNAME_OPT_NAME                    , "Cache Client Name"               , cxxopts::value<std::string>()->default_value(CAOS_CACHECLIENTNAME)                                    )
    (CAOS_CACHEINDEX_OPT_NAME                         , "Cache Index"                     , cxxopts::value<std::uint8_t>()->default_value(std::to_string(CAOS_CACHEINDEX))                        )
    (CAOS_CACHECOMMANDTIMEOUT_OPT_NAME                , "Cache Command Timeout"           , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_CACHECOMMANDTIMEOUT))              )
    (CAOS_CACHEPOOLSIZEMIN_OPT_NAME                   , "Cache Pool Size Min"             , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_CACHEPOOLSIZEMIN))                   )
    (CAOS_CACHEPOOLSIZEMAX_OPT_NAME                   , "Cache Pool Size Max"             , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_CACHEPOOLSIZEMAX))                   )
    (CAOS_CACHEPOOLWAIT_OPT_NAME                      , "Cache Pool Wait"                 , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_CACHEPOOLWAIT))                    )
    (CAOS_CACHEPOOLCONNECTIONTIMEOUT_OPT_NAME         , "Cache Pool Connection Timeout"   , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_CACHEPOOLCONNECTIONTIMEOUT))       )
    (CAOS_CACHEPOOLCONNECTIONLIFETIME_OPT_NAME        , "Cache Pool Connection Lifetime"  , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_CACHEPOOLCONNECTIONLIFETIME))      )
    (CAOS_CACHEPOOLCONNECTIONIDLETIME_OPT_NAME        , "Cache Pool Connection Idle-time" , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_CACHEPOOLCONNECTIONIDLETIME))      )
#endif

    // Database
    (CAOS_DBUSER_OPT_NAME                             , "Database User"                   , cxxopts::value<std::string>()->default_value(CAOS_DBUSER)                                             )
    (CAOS_DBPASS_OPT_NAME                             , "Database Password"               , cxxopts::value<std::string>()->default_value(CAOS_DBPASS)                                             )
    (CAOS_DBHOST_OPT_NAME                             , "Database Host"                   , cxxopts::value<std::string>()->default_value(CAOS_DBHOST)                                             )
    (CAOS_DBPORT_OPT_NAME                             , "Database Port"                   , cxxopts::value<std::uint16_t>()->default_value(std::to_string(CAOS_DBPORT))                           )
    (CAOS_DBNAME_OPT_NAME                             , "Database Name"                   , cxxopts::value<std::string>()->default_value(CAOS_DBNAME)                                             )
    (CAOS_DBPOOLSIZEMIN_OPT_NAME                      , "Database Pool Size Min"          , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBPOOLSIZEMIN))                      )
    (CAOS_DBPOOLSIZEMAX_OPT_NAME                      , "Database Pool Size Max"          , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBPOOLSIZEMAX))                      )
    (CAOS_DBPOOLWAIT_OPT_NAME                         , "Database Pool Wait"              , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_DBPOOLWAIT))                       )
    (CAOS_DBPOOLTIMEOUT_OPT_NAME                      , "Database Pool Timeout"           , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_DBPOOLTIMEOUT))                    )
#ifdef CAOS_USE_DB_POSTGRESQL
    (CAOS_DBKEEPALIVES_OPT_NAME                       , "Database Keepalives"             , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBKEEPALIVES))                       )
    (CAOS_DBKEEPALIVES_IDLE_OPT_NAME                  , "Database Keepalives Idle"        , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBKEEPALIVES_IDLE))                  )
    (CAOS_DBKEEPALIVES_INTERVAL_OPT_NAME              , "Database Keepalives Interval"    , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBKEEPALIVES_INTERVAL))              )
    (CAOS_DBKEEPALIVES_COUNT_OPT_NAME                 , "Database Keepalives Count"       , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBKEEPALIVES_COUNT))                 )
#endif
    (CAOS_DBCONNECT_TIMEOUT_OPT_NAME                  , "Database Connect Timeout"        , cxxopts::value<std::size_t>()->default_value(std::to_string(CAOS_DBCONNECT_TIMEOUT))                  )
    (CAOS_DBMAXWAIT_OPT_NAME                          , "Database Max Wait"               , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_DBMAXWAIT))                        )
    (CAOS_DBHEALTHCHECKINTERVAL_OPT_NAME              , "Database Health Check interval"  , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_DBHEALTHCHECKINTERVAL))            )

    (CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED_OPT_NAME , "Database Health Check interval"  , cxxopts::value<std::uint32_t>()->default_value(std::to_string(CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED))  )
    // (CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE_OPT_NAME      , "Database Healtch Check interval" , cxxopts::value<bool>()->default_value(std::to_string(CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE))                     )
    // (CAOS_VALIDATE_USING_TRANSACTION_OPT_NAME              , "Database Healtch Check interval" , cxxopts::value<bool>()->default_value(std::to_string(CAOS_VALIDATE_USING_TRANSACTION))                             )
  ;

  this->parse();

  this->read();

  SPDLOG_DEBUG("Terminal options parsed");
}



std::string TerminalOptions::help() const noexcept
{
  return options->help();
}


void TerminalOptions::parse()
{
  static constexpr const char* fName = "[caos::TerminalOptions::parse]";

  try
  {
    result = std::make_unique<cxxopts::ParseResult>(options->parse(this->argc, this->argv));
  }
  catch (const cxxopts::exceptions::exception& e)
  {
    throw std::runtime_error("Parsing error: " + std::string(e.what()));
  }
  catch (const std::exception& e)
  {
    SPDLOG_CRITICAL("[{}] : {}",fName,e.what());

    throw;
  }
}



bool TerminalOptions::has(const std::string& option) const noexcept
{
  return (result && result->count(option) > 0);
}



void TerminalOptions::read() const noexcept
{
  if (this->has("help"))
  {
    std::cout << this->help() << "\n";

    #ifndef CAOS_ENABLE_TESTS
    std::exit(0);
    #endif
  }
  else if (this->has("version"))
  {
    std::cout << CAOS_VERSION << "\n";

    #ifndef CAOS_ENABLE_TESTS
    std::exit(0);
    #endif
  }
}
/* -------------------------------------------------------------------------------------------------
 * End of implementation of TerminalOptions
 * -----------------------------------------------------------------------------------------------*/
