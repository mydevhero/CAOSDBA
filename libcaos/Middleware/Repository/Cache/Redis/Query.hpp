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

#include "Redis.hpp"

#ifdef QUERY_EXISTS_IQuery_Example_echoString
std::optional<std::string> Redis::IQuery_Example_echoString(std::string str)
{
  static constexpr const char* fName = "Redis::IQuery_Example_echoString";

  std::optional<std::string> db_result;

  try {
    // 1. Generate la cache key
    std::string cache_key = "echo:" + str;

    // 2. Try getting from Redis
    auto cached_value = this->redis->get(cache_key);
    if (cached_value) {
      spdlog::debug("[{}] Cache hit for key: {}", fName, cache_key);
      std::string cache_result = *cached_value;
      return cache_result + " by Redis";
    }

    spdlog::debug("[{}] Cache miss for key: {}", fName, cache_key);

    if (this->database == nullptr)
    {
      throw std::runtime_error("Database has null object");
    }

    // 3. Cache miss - query database
    db_result = this->database->IQuery_Example_echoString(str);

    // 4. Database return a value, store in cache
    if (db_result.has_value())
    {
      try
      {
        // Store in Redis con TTL (es. 5 minutes)
        this->redis->setex(cache_key,
                    std::chrono::seconds(300),
                    db_result.value());

        spdlog::debug("[{}] Stored in cache with key: {}", fName, cache_key);
      }
      catch (const sw::redis::Error& e)
      {
        // Log dell'errore ma non fallire - la query può comunque restituire il risultato
        spdlog::warn("[{}] Failed to store in cache: {}", fName, e.what());
      }
    }

    return db_result;
  }
  catch (const sw::redis::Error& e)
  {
    // Errore Redis - log e fallback al database
    spdlog::error("[{}] Redis error: {}", fName, e.what());

    // Fallback diretto al database
    if (db_result.has_value())
    {
      return db_result;
    }

    return this->database->IQuery_Example_echoString(str);
  }
  catch (const std::exception& e)
  {
    spdlog::error("[{}] Exception: {}", fName, e.what());
    throw;
  }
  catch(...)
  {
    throw;
  }
};
#endif // End of ifdef QUERY_EXISTS_IQuery_Example_echoString

// std::optional<std::string> Redis::IQuery_Test_sumInt(const int& int1, const int& int2)
// {
//   static constexpr const char* fName = "Redis::IQuery_Example_echoString";

//   std::optional<std::string> db_result;

//   try {
//     // 1. Generate la cache key
//     std::string cache_key = "echo:" + std::to_string(int1+int2);

//     // 2. Try getting from Redis
//     auto cached_value = this->redis->get(cache_key);
//     if (cached_value) {
//       spdlog::debug("[{}] Cache hit for key: {}", fName, cache_key);
//       std::string cache_result = *cached_value;
//       return cache_result + " by Redis";
//     }

//     spdlog::debug("[{}] Cache miss for key: {}", fName, cache_key);

//     if (this->database == nullptr)
//     {
//       throw std::runtime_error("Database has null object");
//     }

//     // 3. Cache miss - query database
//     db_result = this->database->IQuery_Example_echoString(int1, int2);

//     // 4. Database return a value, store in cache
//     if (db_result.has_value())
//     {
//       try
//       {
//         // Store in Redis con TTL (es. 5 minutes)
//         this->redis->setex(cache_key,
//                     std::chrono::seconds(300),
//                     db_result.value());

//         spdlog::debug("[{}] Stored in cache with key: {}", fName, cache_key);
//       }
//       catch (const sw::redis::Error& e)
//       {
//         // Log dell'errore ma non fallire - la query può comunque restituire il risultato
//         spdlog::warn("[{}] Failed to store in cache: {}", fName, e.what());
//       }
//     }

//     return db_result;
//   }
//   catch (const sw::redis::Error& e)
//   {
//     // Errore Redis - log e fallback al database
//     spdlog::error("[{}] Redis error: {}", fName, e.what());

//     // Fallback diretto al database
//     if (db_result.has_value())
//     {
//       return db_result;
//     }

//     return this->database->IQuery_Example_echoString(str);
//   }
//   catch (const std::exception& e)
//   {
//     spdlog::error("[{}] Exception: {}", fName, e.what());
//     throw;
//   }
//   catch(...)
//   {
//     throw;
//   }
// };
