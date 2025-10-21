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

#include "CrowCpp.hpp"


constexpr const char* defaultFinal = "{} : Setting crowcpp {} to {} in {}  environment";









/***************************************************************************************************
 *
 *
 * CrowCpp::CrowCpp() Constructor/Destructor
 *
 *
 **************************************************************************************************/
CrowCpp::CrowCpp()
{
  if (this->terminalPtr==nullptr)
  {
    this->terminalPtr = &TerminalOptions::get_instance();
  }

  this->setHost();
  this->setPort();
  this->setThreadCount();
}
/***************************************************************************************************
 *
 *
 *
 *
 *
 **************************************************************************************************/










// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of CrowCpp::setHost()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CrowCpp::setHost()
{
  static constexpr const char* fName = "CrowCpp::setHost";

  Policy::HostValidator validator("CROWCPP_HOST");

  configureValue<std::string>(
    this->config.host,                              // configField
    &TerminalOptions::get_instance(),               // terminalPtr
    CAOS_CROWCPP_HOST_ENV_NAME,                     // envName
    CAOS_CROWCPP_HOST_OPT_NAME,                     // optName
    "host",                                         // fieldName
    fName,                                          // callerName
    validator,                                      // validator in namespace Policy
    defaultFinal,
    false                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of CrowCpp::setHost()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of CrowCpp::setPort()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CrowCpp::setPort()
{
  static constexpr const char* fName = "CrowCpp::setPort";

  Policy::PortValidator validator;

  configureValue<std::uint16_t>(
    this->config.port,                              // configField
    &TerminalOptions::get_instance(),               // terminalPtr
    CAOS_CROWCPP_PORT_ENV_NAME,               // envName
    CAOS_CROWCPP_PORT_OPT_NAME,               // optName
    "port",                                         // fieldName
    fName,                                          // callerName
    validator,                                      // validator in namespace Policy
    defaultFinal,
    false                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of CrowCpp::setPort()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Init of CrowCpp::setThreadCount()
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CrowCpp::setThreadCount()
{
  static constexpr const char* fName = "CrowCpp::setThreadCount";

  Policy::ThreadsValidator validator;

  configureValue<std::size_t>(
    this->config.threads,                           // configField
    &TerminalOptions::get_instance(),               // terminalPtr
    CAOS_CROWCPP_THREADS_ENV_NAME,                  // envName
    CAOS_CROWCPP_THREADS_OPT_NAME,                  // optName
    "threads",                                      // fieldName
    fName,                                          // callerName
    validator,                                      // validator in namespace Policy
    defaultFinal,
    false                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of CrowCpp::setThreadCount()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



std::string   CrowCpp::getHost()        const noexcept { return this->config.host;    }
std::uint16_t CrowCpp::getPort()        const noexcept { return this->config.port;    }
std::uint16_t CrowCpp::getThreadCount() const noexcept { return this->config.threads; }
/* -------------------------------------------------------------------------------------------------
 * End of class CrowCpp
 * -----------------------------------------------------------------------------------------------*/
