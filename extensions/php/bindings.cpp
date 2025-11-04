extern "C" {
#pragma GCC diagnostic ignored "-Wc++20-extensions"
#include <php.h>
#include <zend_API.h>
#include <zend_modules.h>
#include <ext/standard/info.h>
}

#include <libcaos.hpp>
#include <optional>

std::unique_ptr<Caos>& libcaos(bool check=false)
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (check)
  {
    return caos;
  }

  if (!caos)
  {
    std::vector<std::string> args = {"caos_php"};

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


PHP_FUNCTION(caos_hello);
PHP_FUNCTION(caos_echoString);

// Arginfo
ZEND_BEGIN_ARG_INFO(arginfolibcaos_hello, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfolibcaos_echoString, 0)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

// Exposed functions
static const zend_function_entry caos_functions[] =
{
  PHP_FE(caos_hello, arginfolibcaos_hello)
  PHP_FE(caos_echoString, arginfolibcaos_echoString)
  PHP_FE_END
};

// Init
PHP_MINIT_FUNCTION(caos)
{
  (void)type;
  (void)module_number;
  return SUCCESS;
}

// Shutdown - cleanup
PHP_MSHUTDOWN_FUNCTION(caos)
{
  (void)type;
  (void)module_number;

  try {
    auto& caos = libcaos(true);
    if (caos != nullptr)
    {
      caos.reset();
    }

    return SUCCESS;
  }
  catch (const std::exception& e)
  {
    php_error_docref(NULL, E_ERROR, "CAOS shutdown failed: %s", e.what());
    return FAILURE;
  }

  return SUCCESS;
}

// phpinfo()
PHP_MINFO_FUNCTION(caos)
{
  (void)zend_module;

  php_info_print_table_start();
  php_info_print_table_header(2, "CAOS PHP Extension", "enabled");
  php_info_print_table_end();
}

// Module
zend_module_entry caos_module_entry =
{
  STANDARD_MODULE_HEADER,
  "caos",
  caos_functions,
  PHP_MINIT(caos),
  PHP_MSHUTDOWN(caos),
  NULL,
  NULL,
  PHP_MINFO(caos),
  "0.1",
  STANDARD_MODULE_PROPERTIES
};

extern "C"
{
  ZEND_GET_MODULE(caos)
}




// -------------------------------------------------------------------------------------------------
// caos_hello()
// -------------------------------------------------------------------------------------------------
PHP_FUNCTION(caos_hello)
{
  (void)execute_data;

  RETURN_STRING("Hello from CAOS!\n");
}

// -------------------------------------------------------------------------------------------------
// caos_echoString() -> IQuery_Test_echoString
// -------------------------------------------------------------------------------------------------
PHP_FUNCTION(caos_echoString)
{
  (void)execute_data;

  char *str;
  size_t str_length;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &str, &str_length) == FAILURE)
  {
    RETURN_NULL();
  }

  try
  {
    // Check repository
    Cache* repo = fromRepository();
    if (!repo)
    {
      php_error_docref(NULL, E_WARNING, "CAOS repository not available");
      RETURN_NULL();
    }

    auto repo_result = repo->IQuery_Test_echoString(str);
    if (!repo_result.has_value())
    {
      RETURN_STRING("No result from repository!\n");
    }

    std::string final_result = repo_result.value() + " through PHP!\n";

    RETURN_STRING(final_result.c_str());
  }
  catch (const std::exception& e)
  {
    php_error_docref(NULL, E_WARNING, "CAOS error: %s", e.what());
    RETURN_NULL();
  }
}
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
