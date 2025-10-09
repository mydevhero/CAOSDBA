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

#include <environment.hpp>

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Init of implementation of Environment
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
const std::unordered_map<std::string, Environment::ENV> Environment::ENVMapToString =
{
  {"dev", ENV::dev},
  {"test", ENV::test},
  {"production", ENV::production},
  {"unmanaged", ENV::unmanaged}
};



constexpr const char* Environment::ENV2String(ENV env)
{
  if (env < ENV::EOE)
  {
    return ENVChar.at(static_cast<size_t>(env));
  }

  throw std::out_of_range("ENV is unnknown");
}



inline Environment::ENV Environment::String2ENV(const std::string& envStr)
{
  auto it = ENVMapToString.find(envStr);

  if (it != ENVMapToString.end())
  {
    return it->second;
  }

  throw std::out_of_range("Invalid ENV string: " + envStr);
}



Environment::Environment()
  : terminalPtr(&TerminalOptions::get_instance()), env(ENV::unmanaged), name(ENV2String(env))
{
  SPDLOG_DEBUG("Setting APP environment");

  this->setEnv();                                                                                   // Validate and assign environment
}



Environment::ENV Environment::getEnv()   const noexcept                                             // Get Environment
{
  return this->env;
}



std::string Environment::getName()  const noexcept                                                  // Getter Environment Name
{
  return this->name;
}



void Environment::setEnv()                                                                          // Setr Environment & Environment Name
{
  static constexpr const char* fName = "[Environment::setEnv]";

  try
  {
    if (terminalPtr->has(CAOS_OPT_APP_ENV_NAME))
    {
      this->name = terminalPtr->get<std::string>(CAOS_OPT_APP_ENV_NAME);
    }
    else if (const char* env_name = std::getenv(CAOS_ENV_APP_ENV_NAME))
    {
      this->name = env_name;
    }
    else
    {
      this->name = CAOS_DEFAULT_APP_ENV_VALUE;
    }

    if (this->name.empty())
    {
      throw std::invalid_argument("APP_ENV_NAME empty!");
    }

    SPDLOG_INFO("APP environment is {}",this->name);

    this->env = String2ENV(this->name);                                                             // Validate Environment
  }
  catch(const std::invalid_argument& e)
  {
    SPDLOG_CRITICAL("[{}] : Allowed environment is 'dev', 'test', 'production' - {}",fName,e.what());

    throw;
  }
  catch(const std::out_of_range& e)
  {
    SPDLOG_CRITICAL("[{}] : Allowed environment is 'dev', 'test', 'production' - {}",fName,e.what());

    throw;
  }
  catch (const std::exception& e)
  {
    SPDLOG_CRITICAL("[{}] : {}",fName,e.what());

    throw;
  }

}
/* -------------------------------------------------------------------------------------------------
 * End of Environment
 * -----------------------------------------------------------------------------------------------*/
