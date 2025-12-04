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

#include <Repository.hpp>
#include <memory>
// #include <csignal>

// void signal_handler(int signal)
// {
//   if (signal == SIGINT || signal == SIGTERM)
//   {
//     std::cout << "Signal " << signal << " intercepted. Shutdown running now\n";

//     caos.reset();
//   }
// }

int main(int argc, char* argv[])
{
  // std::signal(SIGINT, signal_handler);
  // std::signal(SIGTERM, signal_handler);

  middleware::Repository middleware{argc, argv};
  crow::App<middleware::Repository> app{middleware};

  CROW_ROUTE(app, "/<string>")([&app](crow::request& req, crow::response& res, std::string str)
  {
    try
    {
      auto& caos = app.get_context<middleware::Repository>(req);

      auto ret = caos.repository->IQuery_Example_echoString(str);

      if (ret.has_value())
      {
        res.set_header("Content-Type", "text/html");
        res.body = ret.value();
        res.code = 200;
      }
      else
      {
        res.set_header("Content-Type", "text/html");
        res.body = "No value";
        res.code = 200;
      }
    }
    catch (const repository::broken_connection& e)
    {
      res.set_header("Content-Type", "text/plain");
      res.body = "Repository unavailable";
      res.code = 503;
    }
    catch (const std::exception& e)
    {
      res.set_header("Content-Type", "text/plain");
      res.body = "Something went wrong";
      res.code = 503;
    }
    catch(...)
    {
      res.set_header("Content-Type", "text/plain");
      res.body = "Unknown exception";
      res.code = 503;
    }

    res.end();
  });

  auto& caos_middleware_instance = app.get_middleware<middleware::Repository>();

  app
  .bindaddr(caos_middleware_instance.caos->crowcpp->getHost())
  .port(caos_middleware_instance.caos->crowcpp->getPort())
  .concurrency(caos_middleware_instance.caos->crowcpp->getThreadCount())
  /*.multithreaded()*/
  .run();

  return 0;
}










// static std::unique_ptr<caos> mycaos;

// void signal_handler(int signal) {
//   if (signal == SIGINT || signal == SIGTERM)
//   {
//     std::cout << "Signal " << signal << " intercepted. Shutdown running now\n";
//     mycaos.reset();
//   }
// }

// int main(int argc, char* argv[])
// {
//   std::signal(SIGINT, signal_handler);
//   std::signal(SIGTERM, signal_handler);

//   spdlog::set_level(CAOS_SEVERITY_LEVEL_BEFORE_LOG_START);

//   SPDLOG_TRACE("Ready to instance caos");

//   mycaos = std::make_unique<caos>(argc, argv);

//   spdlog::info("Avvio applicazione");

//   crow::App<> app;

//   CROW_ROUTE(app, "/<string>")([](crow::response& res, std::string str){

//     spdlog::trace("URL: /<string>");

//     try
//     {
//       auto ret = mycaos->repository->IQuery_Example_echoString(str);

//       if (ret.has_value())
//       {
//         res.set_header("Content-Type", "text/html");
//         res.body = ret.value();
//         res.code = 200;
//       }
//       else
//       {
//         res.set_header("Content-Type", "text/html");
//         res.body = "No value";
//         res.code = 200;
//       }
//     }
//     catch (const repository::broken_connection& e)
//     {
//       res.set_header("Content-Type", "text/plain");
//       res.body = "Repository unavailable";
//       res.code = 503;
//     }
//     catch (const std::exception& e)
//     {
//       res.set_header("Content-Type", "text/plain");
//       res.body = "Something went wrong";
//       res.code = 503;
//     }
//     catch(...)
//     {
//       res.set_header("Content-Type", "text/plain");
//       res.body = "Unknown exception";
//       res.code = 503;
//     }

//     res.end();
//   });

//   app.bindaddr("127.0.0.1").port(18080).concurrency(4)/*multithreaded()*/.run();

//   return 0;
// }
