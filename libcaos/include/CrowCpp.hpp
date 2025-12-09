#pragma once

#include <libcaos/config.hpp>
#include <IRepository.hpp>

/** ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @brief CrowCpp
 * @details Manage and own configuration about network, threads
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
class CrowCpp : public Utils
{
  private:
    struct config_s
    {
      std::string   host      {CAOS_CROWCPP_HOST}     ;
      std::uint16_t port      {CAOS_CROWCPP_PORT}     ;
      std::size_t   threads   {CAOS_CROWCPP_THREADS}  ;
    };

    config_s config;

    void setHost();
    void setPort();
    void setThreadCount();
  public:
    CrowCpp();


    [[nodiscard]] std::string    getHost()         const noexcept;
    [[nodiscard]] std::uint16_t  getPort()         const noexcept;
    [[nodiscard]] std::size_t    getThreadCount()  const noexcept;


    ~CrowCpp(){}

};
/* -------------------------------------------------------------------------------------------------
 * End of Server
 * -----------------------------------------------------------------------------------------------*/
