#pragma once

#include <libcaos.hpp>
#include <Exception.hpp>

#if (defined(CAOS_USE_CROWCPP) || defined (CAOS_CROWCPP_CODE))
#include "crow.h"

namespace middleware
{
  struct Repository
  {
    std::unique_ptr<Caos> caos;

    Repository(int argc, char* argv[])
    {
      caos = std::make_unique<Caos>(argc, argv, initFlags::Repository|initFlags::CrowCpp);
    }

    struct context
    {
      Cache* repository;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx)
    {
      ctx.repository = caos->repository.get();

      if (ctx.repository==nullptr)
      {
        CROW_LOG_ERROR << "Failed to get repository for " << req.url;
        res.code = 503; // Service Unavailable
        res.body = R"({"error": "Repository unavailable"})";
        // res.add_header("Content-Type", "application/json");
        res.end();
        return;
      }

      CROW_LOG_DEBUG << "Acquired repository for " << req.url;
    }

    void after_handle(crow::request&, crow::response& res, context& ctx)
    {
      res.add_header("Cache", "CAOS - Cache App On Steroids");
      ctx.repository = nullptr;
    }
  };
}

#endif // End of ifdef CAOS_USE_CROWCPP
