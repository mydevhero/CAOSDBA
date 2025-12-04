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

// std::unique_ptr<Caos> caos;

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

  auto caos = std::make_unique<Caos>(argc, argv, initFlags::Repository|initFlags::CrowCpp);

  crow::App<> app;

  CROW_ROUTE(app, "/<string>")([&caos](crow::response& res, std::string str)
  {
    try
    {
      auto ret = caos->repository->IQuery_Example_echoString(str);

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

  app
  .bindaddr(caos->crowcpp->getHost())
  .port(caos->crowcpp->getPort())
  .concurrency(caos->crowcpp->getThreadCount())
  /*.multithreaded()*/
  .run();

  return 0;
}
