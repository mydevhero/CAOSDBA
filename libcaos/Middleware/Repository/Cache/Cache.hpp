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

#include "../IRepository.hpp"
#include "generated_queries/Query_Override.hpp"

#ifdef CAOS_USE_CACHE_REDIS
#include <sw/redis++/redis++.h>
#endif

class Cache : public IRepository
{
  private:
    class Pool : public Utils
    {
      private:
        struct config_s
        {
          std::string                                 user                                      ;
          std::string                                 pass                                      ;
          std::string                                 host                  {"127.0.0.1"}       ;
#ifdef CAOS_USE_CACHE_REDIS
          std::uint16_t                               port                  {6379}              ;
#endif
          std::string                                 clientname            {"CAOS"}            ;
          std::uint8_t                                index                 {0}                 ;
          std::chrono::milliseconds                   commandtimeout        {0}                 ;
          std::size_t                                 poolsizemin           {1}                 ;
          std::size_t                                 poolsizemax           {1}                 ;
          std::chrono::milliseconds                   poolwait              {0}                 ;
          std::chrono::milliseconds                   poolconnectiontimeout {0}                 ;
          std::chrono::seconds                        poolconnectionlifetime{0}                 ;
          std::chrono::milliseconds                   poolconnectionidletime{0}                 ;
#ifdef CAOS_USE_CACHE_REDIS
          sw::redis::ConnectionOptions                connection_options                        ;
          sw::redis::ConnectionPoolOptions            pool_options                              ;
#endif
        };
        config_s config;

        // Setters ---------------------------------------------------------------------------------
        void                                          setUser()                                 ;
        void                                          setPass()                                 ;
        void                                          setHost()                                 ;
        void                                          setPort()                                 ;
        void                                          setClientName()                           ;
        void                                          setIndex()                                ;
        void                                          setCommandTimeout()                       ;
        void                                          setPoolSizeMin()                          ;
        void                                          setPoolSizeMax()                          ;
        void                                          setPoolWait()                             ;
        void                                          setPoolConnectionTimeout()                ;
        void                                          setPoolConnectionLifetime()               ;
        void                                          setPoolConnectionIdletime()               ;
#ifdef CAOS_USE_CACHE_REDIS
        void                                          setConnectOpt()                   noexcept;
        void                                          setPoolOpt()                      noexcept;
#endif

        // Getters ---------------------------------------------------------------------------------
        [[nodiscard]] const std::string&              getUser()                   const noexcept;
        [[nodiscard]] const std::string&              getPass()                   const noexcept;
        [[nodiscard]] const std::string&              getHost()                   const noexcept;
        [[nodiscard]] const std::uint16_t&            getPort()                   const noexcept;
        [[nodiscard]] const std::string&              getClientName()             const noexcept;
        [[nodiscard]] const std::uint8_t&             getIndex()                  const noexcept;
        [[nodiscard]] const std::chrono::milliseconds&getCommandTimeout()         const noexcept;
        [[nodiscard]] const std::size_t&              getPoolSizeMin()            const noexcept;
        [[nodiscard]] const std::size_t&              getPoolSizeMax()            const noexcept;
        [[nodiscard]] const std::chrono::milliseconds&getPoolWait()               const noexcept;
        [[nodiscard]] const std::chrono::milliseconds&getPoolConnectionTimeout()  const noexcept;
        [[nodiscard]] const std::chrono::seconds&     getPoolConnectionLifetime() const noexcept;
        [[nodiscard]] const std::chrono::milliseconds&getPoolConnectionIdletime() const noexcept;
#ifdef CAOS_USE_CACHE_REDIS
        [[nodiscard]] const sw::redis::ConnectionOptions getConnectOpt()          const noexcept;
        [[nodiscard]] const sw::redis::ConnectionPoolOptions getPoolOpt()         const noexcept;
#endif

      public:
        Pool():config({})
        {
          if (this->terminalPtr==nullptr)
          {
            this->terminalPtr     = &TerminalOptions::get_instance();
          }

          this->setUser()                   ;
          this->setPass()                   ;
          this->setHost()                   ;
          this->setPort()                   ;
          this->setClientName()             ;
          this->setIndex()                  ;
          this->setCommandTimeout()         ;

          this->setPoolSizeMin()            ;
          this->setPoolSizeMax()            ;
          if (this->getPoolSizeMin() > this->getPoolSizeMax ())
          {
            throw std::out_of_range("CACHEPOOLSIZEMIN > CACHEPOOLSIZEMAX");
          }

          this->setPoolWait()               ;
          this->setPoolConnectionTimeout()  ;
          this->setPoolConnectionLifetime() ;
          this->setPoolConnectionIdletime() ;

#ifdef CAOS_USE_CACHE_REDIS
          this->setConnectOpt()             ;
          this->setPoolOpt()                ;
#endif
        };
        [[nodiscard]] std::unique_ptr<IRepository> init(std::unique_ptr<IRepository>&);
        ~Pool() = default;
    };

    std::unique_ptr<IRepository> database_;
    std::unique_ptr<Pool>        pool;
    std::unique_ptr<IRepository> cache;

  public:
    Cache(std::unique_ptr<IRepository>);

    ~Cache();

    std::unique_ptr<IRepository>& database() { return this->database_; }

    QUERY_OVERRIDE() /* <- from "generated_queries/Query_Override.hpp" */

    // Manually insert your query override here

};







// /*
//  * CAOS - <Cache App On Steroids>
//  * Copyright (C) 2025  Alessandro Bianco
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  *
//  * For support or inquiries, contact <mydevhero@gmail.com>
//  */

// #pragma once

// #include "../IRepository.hpp"

// #ifdef CAOS_USE_CACHE_REDIS
// #include <sw/redis++/redis++.h>
// #endif

// class Cache : public IRepository, public Utils
// {
//   private:
//     struct config_s
//     {
//       std::string                                 user                                      ;
//       std::string                                 pass                                      ;
//       std::string                                 host                  {"127.0.0.1"}       ;
// #ifdef CAOS_USE_CACHE_REDIS
//       std::uint16_t                               port                  {6379}              ;
// #endif
//       std::string                                 clientname            {"CAOS"}            ;
//       std::uint8_t                                index                 {0}                 ;
//       std::chrono::milliseconds                   commandtimeout        {0}                 ;
//       std::size_t                                 poolsizemin           {1}                 ;
//       std::size_t                                 poolsizemax           {1}                 ;
//       std::chrono::milliseconds                   poolwait              {0}                 ;
//       std::chrono::milliseconds                   poolconnectiontimeout {0}                 ;
//       std::chrono::seconds                        poolconnectionlifetime{0}                 ;
//       std::chrono::milliseconds                   poolconnectionidletime{0}                 ;
// #ifdef CAOS_USE_CACHE_REDIS
//       sw::redis::ConnectionOptions                connection_options                        ;
//       sw::redis::ConnectionPoolOptions            pool_options                              ;
// #endif
//     };
//     config_s config;

//     // Setters ---------------------------------------------------------------------------------
//     void                                          setUser()                                 ;
//     void                                          setPass()                                 ;
//     void                                          setHost()                                 ;
//     void                                          setPort()                                 ;
//     void                                          setClientName()                           ;
//     void                                          setIndex()                                ;
//     void                                          setCommandTimeout()                       ;
//     void                                          setPoolSizeMin()                          ;
//     void                                          setPoolSizeMax()                          ;
//     void                                          setPoolWait()                             ;
//     void                                          setPoolConnectionTimeout()                ;
//     void                                          setPoolConnectionLifetime()               ;
//     void                                          setPoolConnectionIdletime()               ;
// #ifdef CAOS_USE_CACHE_REDIS
//     void                                          setConnectOpt()                   noexcept;
//     void                                          setPoolOpt()                      noexcept;
// #endif

//     // Getters ---------------------------------------------------------------------------------
//     [[nodiscard]] const std::string&              getUser()                   const noexcept;
//     [[nodiscard]] const std::string&              getPass()                   const noexcept;
//     [[nodiscard]] const std::string&              getHost()                   const noexcept;
//     [[nodiscard]] const std::uint16_t&            getPort()                   const noexcept;
//     [[nodiscard]] const std::string&              getClientName()             const noexcept;
//     [[nodiscard]] const std::uint8_t&             getIndex()                  const noexcept;
//     [[nodiscard]] const std::chrono::milliseconds&getCommandTimeout()         const noexcept;
//     [[nodiscard]] const std::size_t&              getPoolSizeMin()            const noexcept;
//     [[nodiscard]] const std::size_t&              getPoolSizeMax()            const noexcept;
//     [[nodiscard]] const std::chrono::milliseconds&getPoolWait()               const noexcept;
//     [[nodiscard]] const std::chrono::milliseconds&getPoolConnectionTimeout()  const noexcept;
//     [[nodiscard]] const std::chrono::seconds&     getPoolConnectionLifetime() const noexcept;
//     [[nodiscard]] const std::chrono::milliseconds&getPoolConnectionIdletime() const noexcept;

//     std::unique_ptr<IRepository> cache;
//     std::unique_ptr<IRepository> database_;

//   public:
//     Cache(std::unique_ptr<IRepository>);

//     ~Cache() override;

//     std::unique_ptr<IRepository>& database() { return this->database_; }
// #ifdef CAOS_USE_CACHE_REDIS
//     [[nodiscard]] const sw::redis::ConnectionOptions getConnectOpt()          const noexcept;
//     [[nodiscard]] const sw::redis::ConnectionPoolOptions getPoolOpt()         const noexcept;
// #endif

//     // Queries -------------------------------------------------------------------------------------
//     QUERY_OVERRIDE_LIST()
// };
