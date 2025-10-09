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

#include <libcaos.hpp>
#include <Exception.hpp>
#include "crow.h"

namespace middleware
{
  struct Repository
  {
    std::unique_ptr<Caos> caosPtr;

    Repository(int argc, char* argv[])
    {
      caosPtr = std::make_unique<Caos>(argc, argv, initFlags::Repository);
    }

    struct context
    {
      Cache* repository;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx)
    {
      ctx.repository = caosPtr->repository.get();

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
