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
#include "../Database.hpp"
#include "generated_queries/Query_Override.hpp"

/*
#define CAOS_POSTGRESQL_QUERY_WITH_CONNECTION_GUARD(conn, CODE) \
  ([&]() {                                                      \
    try {                                                       \
      auto result = (CODE)();                                   \
      return result;                                            \
    }                                                           \
    catch (const pqxx::broken_connection& e) {                  \
      if (conn) {                                               \
        this->closeConnection(*((conn).getRaw()));  \
      }                                                         \
      throw PostgreSQL::broken_connection(e.what());            \
    }                                                           \
  }())

#define CAOS_POSTGRESQL_CLOSE_CONNECTION()                      \
  PostgreSQL::Pool::closeConnection(*(connection_opt.value().getRaw()));
*/
class PostgreSQL final: public IRepository
{
  private:
    Database* database;

  public:
    PostgreSQL(Database*);
    ~PostgreSQL() override;

    QUERY_OVERRIDE() /* <- from "generated_queries/Query_Override.hpp" */

    // Manually insert your query override here
};





