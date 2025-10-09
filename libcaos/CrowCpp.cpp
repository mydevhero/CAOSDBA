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
#include <arpa/inet.h>                                                                              // Validate IP address

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class CrowCpp
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
CrowCpp::CrowCpp()
{
  this->config = { std::make_unique<CrowCpp::Network>(), std::make_unique<CrowCpp::Thread>() };
}

std::string                 CrowCpp::getAddress()       const noexcept { return this->config.network->getAddress();        }
std::uint16_t               CrowCpp::getPort()          const noexcept { return this->config.network->getPort();           }
std::uint16_t               CrowCpp::getThreadCount()   const noexcept { return this->config.thread->get();                }
CrowCpp::Network::IPVersion CrowCpp::getIPVersion()     const noexcept { return this->config.network->getIPVersion();      }
const char*                 CrowCpp::getIPVersionName() const          { return this->config.network->getIPVersionName();  }
/* -------------------------------------------------------------------------------------------------
 * class CrowCpp
 * -----------------------------------------------------------------------------------------------*/


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class CrowCpp::Network
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
constexpr const char* CrowCpp::Network::IPVersionName(IPVersion ipv)
{
  if (ipv < IPVersion::EOE)
  {
    return IPVersionChar[static_cast<uint8_t>(ipv)];
  }

  throw std::out_of_range("IP protocol is unmanaged");
}



CrowCpp::Network::Network()
{
  if (this->terminalPtr==nullptr)
  {
    this->terminalPtr     = &TerminalOptions::get_instance();
  }

  if (this->environmentRef==nullptr)
  {
    this->environmentRef  = &Environment::get_instance();
  }

  spdlog::trace("Running Network constructor");

  this->setAddress(); // Set ipversion as well
  this->setPort();
}



std::string                 CrowCpp::Network::getAddress()       const noexcept { return this->address;                   }
std::uint16_t               CrowCpp::Network::getPort()          const noexcept { return this->port;                      }
CrowCpp::Network::IPVersion CrowCpp::Network::getIPVersion()     const noexcept { return this->ipversion;                 }
const char*                 CrowCpp::Network::getIPVersionName() const          { return IPVersionName(this->ipversion);  }

void CrowCpp::Network::setAddress()
{
  spdlog::trace("Setting network address");

  static constexpr const char* fName = "CrowCpp::Network::setAddress";

  try
  {
    // Set primary bind to address for production environment --------------------------------------
    if (TerminalOptions::get_instance().has(CAOS_OPT_CROWCPP_BINDTOADDRESS_NAME))
    {
      this->address = TerminalOptions::get_instance().get<std::string>(CAOS_OPT_CROWCPP_BINDTOADDRESS_NAME);
    }
    else if (const char* envAddrStr = std::getenv(CAOS_ENV_CROWCPP_BINDTOADDRESS_NAME))
    {
      this->address = envAddrStr;
    }
    else
    {
#ifdef CAOS_DEFAULT_CROWCPP_BINDTOADDRESS
      this->address = CAOS_DEFAULT_CROWCPP_BINDTOADDRESS;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative bind to address for test or dev environment ---------------------------------
    if (this->isDevOrTestEnv())
    {
#ifdef CAOS_DEFAULT_CROWCPP_BINDTOADDRESS_ENV_DEV_OR_TEST
      this->address = CAOS_DEFAULT_CROWCPP_BINDTOADDRESS_ENV_DEV_OR_TEST;
#endif
    }
    // ---------------------------------------------------------------------------------------------



    struct sockaddr_in sa4={};
    struct sockaddr_in6 sa6={};

    // Validate ipV4
    if (inet_pton(AF_INET, this->address.c_str(), &(sa4.sin_addr)) == 1)
    {
      this->ipversion = IPVersion::IPv4;

      spdlog::trace("Network address (IPv4) is: {}",this->address);

      return;
    }

    // Validate ipV6
    if (inet_pton(AF_INET6, this->address.c_str(), &(sa6.sin6_addr)) == 1)
    {
      this->ipversion = IPVersion::IPv6;

      spdlog::trace("Network address (IPv6) is: {}",this->address);

      return;
    }

    throw std::invalid_argument("Invalid Address: " + this->address);
  }
  catch(const std::invalid_argument& e)
  {
    spdlog::critical("[{}] : {}", fName, e.what());
    throw;
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}", fName, e.what());
    throw;
  }
}



void CrowCpp::Network::setPort()
{
  spdlog::trace("Setting CrowCpp network port");

  static constexpr const char* fName = "CrowCpp::Network::setPort";

  try
  {
    // Set primary bind to port for production environment -----------------------------------------
    if (TerminalOptions::get_instance().has(CAOS_OPT_CROWCPP_BINDTOPORT_NAME))
    {
      this->port = TerminalOptions::get_instance().get<std::uint16_t>(CAOS_OPT_CROWCPP_BINDTOPORT_NAME);
    }
    else if (const char* envPortStr = std::getenv(CAOS_ENV_CROWCPP_BINDTOPORT_NAME))
    {
      this->port = static_cast<std::uint16_t>(std::stoi(envPortStr));
    }
    else
    {
#if defined(CAOS_DEFAULT_CROWCPP_BINDTOPORT) && (CAOS_DEFAULT_CROWCPP_BINDTOPORT + 0) > 0
      this->port = static_cast<std::uint16_t>(CAOS_DEFAULT_CROWCPP_BINDTOPORT);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    // Set alternative bind to port for test or dev environment ------------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CROWCPP_BINDTOPORT_ENV_DEV_OR_TEST) && (CAOS_DEFAULT_CROWCPP_BINDTOPORT_ENV_DEV_OR_TEST + 0) > 0
      this->port = static_cast<std::uint16_t>(CAOS_DEFAULT_CROWCPP_BINDTOPORT_ENV_DEV_OR_TEST);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    if(this->port < unprivileged_port_min || this->port > unprivileged_port_max)
    {
      throw std::out_of_range("TCP Port out of range: " + std::to_string(this->port));
    }

    spdlog::trace("Network port is: {}", this->port);
  }
  catch(const std::out_of_range& e)
  {
    spdlog::critical("[{}] : [TCP Port range from {} to {}] : {}", fName, unprivileged_port_min, unprivileged_port_max, e.what());
    throw;
  }
  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}", fName, e.what());
    throw;
  }
}
/* -------------------------------------------------------------------------------------------------
 * class CrowCpp::Network
 * -----------------------------------------------------------------------------------------------*/






/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class CrowCpp::Thread
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
CrowCpp::Thread::Thread() : threads(0)
{
  if (this->terminalPtr==nullptr)
  {
    this->terminalPtr     = &TerminalOptions::get_instance();
  }

  if (this->environmentRef==nullptr)
  {
    this->environmentRef  = &Environment::get_instance();
  }

  this->set();
}



std::uint16_t CrowCpp::Thread::get() const noexcept { return this->threads; }



void CrowCpp::Thread::set()                                                                         // Set Thread count
{
  spdlog::trace("Setting threads count");

  static constexpr const char* fName = "CrowCpp::Thread::set";

  try
  {
    // Set primary threads value for production environment ----------------------------------------
    if (TerminalOptions::get_instance().has(CAOS_OPT_CROWCPP_THREADS_NAME))
    {
      this->threads = TerminalOptions::get_instance().get<std::uint16_t>(CAOS_OPT_CROWCPP_THREADS_NAME);
    }
    else if (const char* threadsStr = std::getenv(CAOS_ENV_CROWCPP_THREADS_NAME))
    {
      this->threads = static_cast<std::uint16_t>(std::stoi(threadsStr));
    }
    else
    {
#if defined(CAOS_DEFAULT_CROWCPP_THREADS) && (CAOS_DEFAULT_CROWCPP_THREADS + 0) > 0
      this->threads = static_cast<std::uint16_t>(CAOS_DEFAULT_CROWCPP_THREADS);
    }
#endif
    // ---------------------------------------------------------------------------------------------



    // Set alternative threads value for test or dev environment -----------------------------------
    if (this->isDevOrTestEnv())
    {
#if defined(CAOS_DEFAULT_CROWCPP_THREADS_ENV_DEV_OR_TEST) && (CAOS_DEFAULT_CROWCPP_THREADS_ENV_DEV_OR_TEST + 0) > 0
      this->threads = static_cast<std::uint16_t>(CAOS_DEFAULT_CROWCPP_THREADS_ENV_DEV_OR_TEST);
#endif
    }
    // ---------------------------------------------------------------------------------------------



    auto max_threads = (std::thread::hardware_concurrency() * 2);

    if (this->threads < 1 || this->threads > max_threads)
    {
      spdlog::warn("Setting CROWCPP_THREADS to {}", max_threads);

      this->threads = static_cast<std::uint16_t>(max_threads);
    }

    spdlog::trace("Threads count is: {}", this->threads);
  }

  catch (const std::exception& e)
  {
    spdlog::critical("[{}] : {}", fName, e.what());
    std::exit(1);
  }
}
/* -------------------------------------------------------------------------------------------------
 * class CrowCpp::Thread
 * -----------------------------------------------------------------------------------------------*/
