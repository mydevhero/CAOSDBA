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

#include "PostgreSQL.hpp"

std::optional<std::string> PostgreSQL::IQuery_Test_echoString(std::string str)
{
  if(!running.load(std::memory_order_relaxed))
  {
    return std::nullopt;
  }

  auto connection_opt = this->database->acquire();

  try
  {
    if (connection_opt)
    {
      Database::ConnectionWrapper& connection = connection_opt.value();

      pqxx::result result;

      {
        pqxx::work tx(*connection);
        pqxx::params p;
        p.append(str);
        result = tx.exec("SELECT  $1", p);/*pg_sleep(0.05),*/
        tx.commit();
      }

      if (!result.empty())
      {
        return result[0][0].as<std::string>() + " from PostgreSQL";
      }

      return std::nullopt;
    }

    throw repository::broken_connection("Database connection unavailable - cannot acquire connection from pool");

  }
  catch (const repository::broken_connection& e)
  {
    throw;
  }
  catch (const pqxx::sql_error& e)
  {
    throw;
  }
  catch (const std::exception& e)
  {
    throw;
  }
  catch(...)
  {
    throw;
  }

  return std::nullopt;
}

// std::optional<std::string> PostgreSQL::IQuery_Test_sumInt(const int& int1, const int& int2)
// {
//   if(!running.load(std::memory_order_relaxed))
//   {
//     return std::nullopt;
//   }

//   auto connection_opt = this->database->acquire();

//   try
//   {
//     if (connection_opt)
//     {
//       Database::ConnectionWrapper& connection = connection_opt.value();

//       pqxx::result result;

//       {
//         pqxx::work tx(*connection);
//         pqxx::params p;
//         p.append(int1);
//         p.append(int2);
//         result = tx.exec("SELECT  $1+$2", p);/*pg_sleep(0.05),*/
//         tx.commit();
//       }

//       if (!result.empty())
//       {
//         return result[0][0].as<std::string>() + " from PostgreSQL";
//       }

//       return std::nullopt;
//     }

//     throw repository::broken_connection("Database connection unavailable - cannot acquire connection from pool");

//   }
//   catch (const repository::broken_connection& e)
//   {
//     throw;
//   }
//   catch (const pqxx::sql_error& e)
//   {
//     throw;
//   }
//   catch (const std::exception& e)
//   {
//     throw;
//   }
//   catch(...)
//   {
//     throw;
//   }

//   return std::nullopt;
// }
