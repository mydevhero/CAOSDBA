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





