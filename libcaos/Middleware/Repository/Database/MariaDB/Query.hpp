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

#include "MariaDB.hpp"

std::optional<std::string> MariaDB::IQuery_Test_echoString(std::string str)
{
  static constexpr const char* fName = "MariaDB::echoString";

  if(!running_.load(std::memory_order_relaxed))
  {
    return std::nullopt;
  }

  auto connection_opt = this->database->acquire();

  try
  {
    if (connection_opt)
    {
      Database::ConnectionWrapper& connection = connection_opt.value();

      // Disable autocommit mocking a transaction
      connection->setAutoCommit(false);

      try
      {
        std::unique_ptr<sql::PreparedStatement> pstmt(
          connection->prepareStatement("SELECT ? as echoed_string")
        );

        pstmt->setString(1, str);
        std::unique_ptr<sql::ResultSet> result(pstmt->executeQuery());

        std::optional<std::string> returnValue = std::nullopt;

        if (result->next())
        {
          returnValue = result->getString("echoed_string") + " from MariaDB";
        }

        // Explicit commit
        connection->commit();
        connection->setAutoCommit(true);

        return returnValue;
      }
      catch (...)
      {
        // Rollback
        try { connection->rollback(); } catch (...) {}
        try { connection->setAutoCommit(true); } catch (...) {}
        throw;
      }
    }

    throw repository::broken_connection("Database connection unavailable - cannot acquire connection from pool");
  }
  catch (const repository::broken_connection& e)
  {
    throw;
  }
  catch (sql::SQLException& e)
  {
    std::uint32_t errorCode = e.getErrorCode();

    if (errorCode == 2002 ||
        errorCode == 2003 ||
        errorCode == 2006 ||
        errorCode == 2013 ||
        errorCode == 1927)
    {
      throw repository::broken_connection("MariaDB connection broken: " + std::string(e.what()));
    }

    spdlog::error("[{}] MariaDB error [{}:{}]: {}", fName, errorCode, e.getSQLState(), e.what());

    throw std::runtime_error("MariaDB database error");
  }
  catch (const std::exception& e)
  {
    spdlog::error("[{}] Exception: {}", fName, e.what());
    throw;
  }
  catch(...)
  {
    spdlog::error("[{}] Unknown exception", fName);
    throw std::runtime_error("Unknown error in echoString");
  }

  return std::nullopt;
}
