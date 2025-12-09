#pragma once

#include <libcaos/config.hpp>
#include "../Database.hpp"
#include "generated_queries/Query_Override.hpp"

class MySQL final: public IRepository
{
  private:
    Database* database;

  public:

    MySQL(Database*);

    ~MySQL() override;

    QUERY_OVERRIDE() /* <- from "generated_queries/Query_Override.hpp" */

    // Manually insert your query override here
};





