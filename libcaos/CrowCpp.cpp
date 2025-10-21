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
  const char* fName     = "CrowCpp::setHost"                        ;
  const char* fieldName = "CROWCPP_HOST"                            ;
  using       dataType  = std::string                               ;

  Policy::HostValidator validator(fieldName)                        ;

  configureValue<dataType>(
    this->config.host,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CROWCPP_HOST_ENV_NAME,                                     // envName
    CAOS_CROWCPP_HOST_OPT_NAME,                                     // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
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
  const char* fName     = "CrowCpp::setPort"                        ;
  const char* fieldName = "CROWCPP_PORT"                            ;
  using       dataType  = std::uint16_t                             ;

  Policy::PortValidator validator                                   ;

  configureValue<dataType>(
    this->config.port,                                              // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CROWCPP_PORT_ENV_NAME,                                     // envName
    CAOS_CROWCPP_PORT_OPT_NAME,                                     // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
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
  const char* fName     = "CrowCpp::setThreadCount"                 ;
  const char* fieldName = "CROWCPP_THREADS"                         ;
  using       dataType  = std::size_t                               ;

  Policy::ThreadsValidator validator                                ;

  configureValue<dataType>(
    this->config.threads,                                           // configField
    &TerminalOptions::get_instance(),                               // terminalPtr
    CAOS_CROWCPP_THREADS_ENV_NAME,                                  // envName
    CAOS_CROWCPP_THREADS_OPT_NAME,                                  // optName
    fieldName,                                                      // fieldName
    fName,                                                          // callerName
    validator,                                                      // validator in namespace Policy
    defaultFinal,
    false                                                           // exitOnError
  );
}
// -------------------------------------------------------------------------------------------------
// End of CrowCpp::setThreadCount()
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------



std::string    CrowCpp::getHost()        const noexcept { return this->config.host;    }
std::uint16_t  CrowCpp::getPort()        const noexcept { return this->config.port;    }
std::size_t    CrowCpp::getThreadCount() const noexcept { return this->config.threads; }
/* -------------------------------------------------------------------------------------------------
 * End of class CrowCpp
 * -----------------------------------------------------------------------------------------------*/
