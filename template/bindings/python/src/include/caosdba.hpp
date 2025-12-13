#pragma once

#include <Python.h>
#include <libcaos.hpp>
#include <build_info.h>

// =================================================================================================
// GLOBAL STATE AND INITIALIZATION
// =================================================================================================

/**
 * Get or initialize the global CAOS instance
 */
std::unique_ptr<Caos>& libcaos(bool check = false)
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (check)
  {
    return caos;
  }

  if (!caos)
  {
    std::vector<std::string> args = {PYTHON_MODULE_NAME_STR};

    if (const char* config_path = std::getenv("CAOS_CONFIG"))
    {
      args.push_back("--config");
      args.push_back(config_path);
    }

    std::vector<char*> argv;
    for (auto& arg : args)
    {
      argv.push_back(&arg[0]);
    }

    caos = std::make_unique<Caos>(argv.size(), argv.data(), (initFlags::Repository));
  }

  return caos;
}

/**
 * Get the CAOS repository/cache instance
 */
Cache* Repository()
{
  auto& caos = libcaos();
  return caos ? caos->repository.get() : nullptr;
}

/**
 * Cleanup function registered with Python atexit
 */
static void caos_shutdown()
{
  try
  {
    auto& caos = libcaos(true);
    if (caos)
    {
      caos.reset();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "CAOS Python shutdown error: " << e.what() << std::endl;
  }
}
