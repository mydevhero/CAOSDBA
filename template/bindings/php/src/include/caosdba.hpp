#pragma once

#include <php.h>
#include <libcaos.hpp>
#include <build_info.h>

/**
 * Get or initialize the global CAOS instance
 *
 * @param check If true, only check if instance exists without initializing
 * @return Reference to the unique_ptr holding the CAOS instance
 *
 * The CAOS instance is initialized lazily on first access with:
 * - Command line arguments from environment (CAOS_CONFIG if set)
 * - Repository initialization flag
 */
std::unique_ptr<Caos>& libcaos(bool check=false)
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (check)
  {
    return caos;
  }

  if (!caos)
  {
    // Build fake command line arguments
    std::vector<std::string> args = {"caos_php"};
    std::vector<char*> argv;
    for (auto& arg : args)
    {
      argv.push_back(&arg[0]);
    }
    int argc = argv.size();

    // Initialize CAOS with repository
    caos = std::make_unique<Caos>(argc, argv.data(), (initFlags::Repository));
  }

  return caos;
}

/**
 * Get the CAOS repository/cache instance
 *
 * @return Pointer to Cache instance, or nullptr if not available
 */
Cache* Repository()
{
  auto& caos = libcaos();
  return caos ? caos->repository.get() : nullptr;
}

/**
 * Cleanup function for CAOS library
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
    // Log error if needed
  }
}
