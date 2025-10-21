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
