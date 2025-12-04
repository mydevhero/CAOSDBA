#include <libcaos.hpp>
#include <memory>
#include <vector>

#include <napi.h>

std::unique_ptr<Caos>& libcaos()
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (!caos)
  {
    std::vector<std::string> args = {"caos_node"};

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
    int argc = argv.size();

    caos = std::make_unique<Caos>(argc, argv.data(), initFlags::Repository);
  }

  return caos;
}

Cache* fromRepository()
{
  auto& caos = libcaos();
  return caos ? caos->repository.get() : nullptr;
}

// -------------------------------------------------------------------------------------------------
// Cleanup function
// -------------------------------------------------------------------------------------------------
static void caos_shutdown(void*)
{
  try {
    auto& caos = libcaos();
    if (caos) {
      caos.reset();
      // printf("CAOS Node.js extension shutdown completed\n");
    }
  }
  catch (const std::exception& e) {
    fprintf(stderr, "CAOS shutdown failed: %s\n", e.what());
  }
}


// -------------------------------------------------------------------------------------------------
// caos_hello()
// -------------------------------------------------------------------------------------------------
Napi::String caos_hello(const Napi::CallbackInfo& info)
{
  return Napi::String::New(info.Env(), "Hello from CAOS!\n");
}

// -------------------------------------------------------------------------------------------------
// caos_echoString()
// -------------------------------------------------------------------------------------------------
Napi::String caos_echoString(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  try
  {
    std::string input = info[0].As<Napi::String>();
    Cache* repo = fromRepository();

    if (!repo)
    {
      return Napi::String::New(env, "CAOS repository not available!\n");
    }

    auto repo_result = repo->IQuery_Example_echoString(input);
    if (!repo_result.has_value())
    {
      return Napi::String::New(env, "No result from repository!\n");
    }

    std::string final_result = repo_result.value() + " through Node.js!\n";
    return Napi::String::New(env, final_result);

  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, "CAOS error: " + std::string(e.what())).ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{

  fromRepository();

   napi_add_env_cleanup_hook(env, caos_shutdown, nullptr);

  exports.Set("hello", Napi::Function::New(env, caos_hello));
  exports.Set("echoString", Napi::Function::New(env, caos_echoString));

  return exports;
}

NODE_API_MODULE(caos, Init)
