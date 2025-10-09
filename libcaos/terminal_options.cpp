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
TerminalOptions::TerminalOptions(int argc, char** argv) : argc(argc), argv(argv)
{
  options = std::make_unique<cxxopts::Options>(argv[0], CAOS_OFFICIALNAME);

  options->add_options()
    ("help"                                     , "Show help"                                               )
    ("version"                                  , "Get build version&timestamp"                             )

    // Environment
    (CAOS_OPT_APP_ENV_NAME                      , "Environment"                         , cxxopts::value<std::string>()   )

    // Log
    (CAOS_OPT_LOG_SEVERITY_NAME                 , "Log Terminal Severity"               , cxxopts::value<std::string>()   )

    // CrowCpp
#ifdef CAOS_USE_CROWCPP
    (CAOS_OPT_CROWCPP_BINDTOADDRESS_NAME        , "Bind to address"                     , cxxopts::value<std::string>()   )
    (CAOS_OPT_CROWCPP_BINDTOPORT_NAME           , "Bind to port"                        , cxxopts::value<std::uint16_t>() )
    (CAOS_OPT_CROWCPP_THREADS_NAME              , "Number of threads"                   , cxxopts::value<std::uint16_t>() )
#endif

    // Cache
#ifdef CAOS_USE_CACHE
    (CAOS_OPT_CACHEUSER_NAME                    , "Cache User"                          , cxxopts::value<std::string>()   )
    (CAOS_OPT_CACHEPASS_NAME                    , "Cache Password"                      , cxxopts::value<std::string>()   )
    (CAOS_OPT_CACHEHOST_NAME                    , "Cache Host"                          , cxxopts::value<std::string>()   )
    (CAOS_OPT_CACHEPORT_NAME                    , "Cache Port"                          , cxxopts::value<std::uint16_t>() )
    (CAOS_OPT_CACHECLIENTNAME_NAME              , "Cache Client Name"                   , cxxopts::value<std::string>()   )
    (CAOS_OPT_CACHEINDEX_NAME                   , "Cache Index"                         , cxxopts::value<std::uint8_t>()  )
    (CAOS_OPT_CACHECOMMANDTIMEOUT_NAME          , "Cache Command Timeout"               , cxxopts::value<std::uint32_t>() )
    (CAOS_OPT_CACHEPOOLSIZEMIN_NAME             , "Cache Pool Size Min"                 , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_CACHEPOOLSIZEMAX_NAME             , "Cache Pool Size Max"                 , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_CACHEPOOLWAIT_NAME                , "Cache Pool Wait"                     , cxxopts::value<std::uint32_t>() )
    (CAOS_OPT_CACHEPOOLCONNECTIONTIMEOUT_NAME   , "Cache Pool Connection Timeout"       , cxxopts::value<std::uint32_t>() )
    (CAOS_OPT_CACHEPOOLCONNECTIONLIFETIME_NAME  , "Cache Pool Connection Lifetime"      , cxxopts::value<std::uint32_t>() )
    (CAOS_OPT_CACHEPOOLCONNECTIONIDLETIME_NAME  , "Cache Pool Connection Idle Lifetime" , cxxopts::value<std::uint32_t>() )
#endif

    // Database
    (CAOS_OPT_DBUSER_NAME                       , "Database User"                       , cxxopts::value<std::string>()   )
    (CAOS_OPT_DBPASS_NAME                       , "Database Password"                   , cxxopts::value<std::string>()   )
    (CAOS_OPT_DBHOST_NAME                       , "Database Host"                       , cxxopts::value<std::string>()   )
    (CAOS_OPT_DBPORT_NAME                       , "Database Port"                       , cxxopts::value<std::uint16_t>() )
    (CAOS_OPT_DBNAME_NAME                       , "Database Name"                       , cxxopts::value<std::string>()   )
    (CAOS_OPT_DBPOOLSIZEMIN_NAME                , "Database Pool Size Min"              , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_DBPOOLSIZEMAX_NAME                , "Database Pool Size Max"              , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_DBPOOLWAIT_NAME                   , "Database Pool Wait"                  , cxxopts::value<std::uint32_t>() )
#ifdef CAOS_USE_DB_POSTGRESQL
    (CAOS_OPT_DBKEEPALIVES_NAME                 , "Database Keepalives"                 , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_DBKEEPALIVES_IDLE_NAME            , "Database Keepalives Idle"            , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_DBKEEPALIVES_INTERVAL_NAME        , "Database Keepalives Interval"        , cxxopts::value<std::size_t>()   )
    (CAOS_OPT_DBKEEPALIVES_COUNT_NAME           , "Database Keepalives Count"           , cxxopts::value<std::size_t>()   )
#endif
    (CAOS_OPT_DBMAXWAIT_NAME                    , "Database Max Wait"                   , cxxopts::value<std::uint32_t>() )
    (CAOS_OPT_DBHEALTHCHECKINTERVAL_NAME        , "Database Healtch Check interval"     , cxxopts::value<std::uint32_t>() )
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
