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
class CrowCpp
{
  private:
    // Threads configuration manager ---------------------------------------------------------------
    class Thread : public Utils
    {
      private:
        std::uint16_t           threads = (std::thread::hardware_concurrency() * 2);

        void                    set();

      public:
         Thread();

        [[nodiscard]] std::uint16_t get() const noexcept;
    };
    // ---------------------------------------------------------------------------------------------



    // Network configuration manager ---------------------------------------------------------------
    class Network : public Utils
    {
      public:
        enum class IPVersion:std::uint8_t { IPv4=0, IPv6, unmanaged, EOE };                         // EOE = End Of Enum

      private:
        std::string   address   = "127.0.0.1";
        std::uint16_t port      = 18080;
        IPVersion     ipversion = IPVersion::IPv4;

        void setAddress();
        void setPort();

        static constexpr std::array<const char*, static_cast<std::size_t>(IPVersion::EOE)>
          IPVersionChar = { "IPv4", "IPv6", "unmanaged" };

        static constexpr inline const char* IPVersionName(IPVersion ipv);
      public:

         Network();

        [[nodiscard]] std::string   getAddress()        const noexcept;
        [[nodiscard]] std::uint16_t getPort()           const noexcept;
        [[nodiscard]] IPVersion     getIPVersion()      const noexcept;
        [[nodiscard]] const char*   getIPVersionName()  const;
    };
    // ---------------------------------------------------------------------------------------------

    struct config_s
    {
      std::unique_ptr<Network>     network;
      std::unique_ptr<Thread>      thread;
    };

    config_s config;

  public:
    CrowCpp();

    [[nodiscard]] std::string         getAddress()        const noexcept;
    [[nodiscard]] std::uint16_t       getPort()           const noexcept;
    [[nodiscard]] std::uint16_t       getThreadCount()    const noexcept;
    [[nodiscard]] Network::IPVersion  getIPVersion()      const noexcept;
    [[nodiscard]] const char*         getIPVersionName()  const;

    ~CrowCpp()
    {
      spdlog::trace("Destroying CrowCpp");

      this->config.network.reset();
      this->config.thread.reset();

      spdlog::trace("CrowCpp Destroyed");
    }

};
/* -------------------------------------------------------------------------------------------------
 * End of Server
 * -----------------------------------------------------------------------------------------------*/
