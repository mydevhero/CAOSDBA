#pragma once

#include <chrono>
#include <memory>
#include "../Cache.hpp"
#include "generated_queries/Query_Override.hpp"

class Redis final : public IRepository
{
  public:
    Redis(std::unique_ptr<IRepository>&, const sw::redis::ConnectionOptions&, const sw::redis::ConnectionPoolOptions&);

    ~Redis() = default;

    QUERY_OVERRIDE() /* <- from "generated_queries/Query_Override.hpp" */

    // Manually insert your query override here

  private:
    Cache*                        cache;
    std::unique_ptr<IRepository>& database;
    std::unique_ptr<sw::redis::Redis> redis;
};
