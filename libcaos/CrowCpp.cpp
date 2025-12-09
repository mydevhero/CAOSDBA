#include "CrowCpp.hpp"


constexpr const char* defaultFinal = "{} : Setting crowcpp {} to {} in {} environment";









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

  Policy::PortValidator validator("CROWCPP_PORT")                   ;

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

  Policy::ThreadsValidator validator("CROWCPP_THREADS")             ;

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
