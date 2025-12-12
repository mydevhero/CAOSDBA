/**
 * CAOS Python Extension
 *
 * Python binding for CAOS library with integrated call_context validation system.
 * All query functions require a call_context dictionary as first parameter.
 * Token is optional - validation only occurs if token is present.
 */

#include <Python.h>
#include <libcaos.hpp>
#include <memory>
#include <vector>
#include <iostream>

#include "extension_config.h"  // Generated configuration
#include "call_context.hpp"    // Call context management

// =================================================================================================
// GLOBAL STATE AND INITIALIZATION
// =================================================================================================

/**
 * Get or initialize the global CAOS instance
 */
std::unique_ptr<Caos>& libcaos(bool check = false)
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (check) {
    return caos;
  }

  if (!caos) {
    std::vector<std::string> args = {PYTHON_MODULE_NAME_STR};

    if (const char* config_path = std::getenv("CAOS_CONFIG")) {
      args.push_back("--config");
      args.push_back(config_path);
    }

    std::vector<char*> argv;
    for (auto& arg : args) {
      argv.push_back(&arg[0]);
    }

    caos = std::make_unique<Caos>(argv.size(), argv.data(), (initFlags::Repository));
  }

  return caos;
}

/**
 * Get the CAOS repository/cache instance
 */
Cache* fromRepository()
{
  auto& caos = libcaos();
  return caos ? caos->repository.get() : nullptr;
}

/**
 * Cleanup function registered with Python atexit
 */
static void caos_shutdown()
{
  try {
    auto& caos = libcaos(true);
    if (caos) {
      caos.reset();
    }
  } catch (const std::exception& e) {
    std::cerr << "CAOS Python shutdown error: " << e.what() << std::endl;
  }
}

// =================================================================================================
// ERROR HANDLING UTILITIES
// =================================================================================================

/**
 * Create standardized Python error object
 */
static PyObject* create_error_object(const std::string& error_type,
                                     const std::string& error_message,
                                     PyObject* additional_data = nullptr)
{
  PyObject* dict = PyDict_New();
  if (!dict) {
    return nullptr;
  }

  // success: false
  PyDict_SetItemString(dict, "success", Py_False);

  // error_type
  PyObject* error_type_str = PyUnicode_FromString(error_type.c_str());
  if (!error_type_str) {
    Py_DECREF(dict);
    return nullptr;
  }
  PyDict_SetItemString(dict, "error_type", error_type_str);
  Py_DECREF(error_type_str);

  // error_message
  PyObject* error_msg_str = PyUnicode_FromString(error_message.c_str());
  if (!error_msg_str) {
    Py_DECREF(dict);
    return nullptr;
  }
  PyDict_SetItemString(dict, "error_message", error_msg_str);
  Py_DECREF(error_msg_str);

  // data
  if (additional_data) {
    Py_INCREF(additional_data);
    PyDict_SetItemString(dict, "data", additional_data);
  } else {
    PyDict_SetItemString(dict, "data", Py_None);
    Py_INCREF(Py_None);
  }

  return dict;
}

/**
 * Create standardized Python success object
 */
static PyObject* create_success_object(PyObject* data)
{
  PyObject* dict = PyDict_New();
  if (!dict) {
    return nullptr;
  }

  // success: true
  PyDict_SetItemString(dict, "success", Py_True);

  // data
  if (data) {
    Py_INCREF(data);
    PyDict_SetItemString(dict, "data", data);
  } else {
    PyDict_SetItemString(dict, "data", Py_None);
    Py_INCREF(Py_None);
  }

  return dict;
}

/**
 * Helper for parameter parsing errors
 */
static PyObject* create_parameter_error(const std::string& message)
{
  return create_error_object("PARAMETER", message);
}

/**
 * Helper for system errors
 */
static PyObject* create_system_error(const std::string& message)
{
  return create_error_object("SYSTEM", message);
}

// =================================================================================================
// QUERY IMPLEMENTATION - IQuery_Template_echoString
// =================================================================================================

/**
 * IQuery_Template_echoString - Echo string through CAOS repository
 */
static PyObject* IQuery_Template_echoString(PyObject* self, PyObject* args)
{
  PyObject* call_context_obj;
  char* input_str;

  // Parse arguments: call_context (O) + string (s)
  if (!PyArg_ParseTuple(args, "Os", &call_context_obj, &input_str)) {
    return create_parameter_error("Invalid parameters for IQuery_Template_echoString");
  }

  // Verify call_context is a dictionary
  if (!PyDict_Check(call_context_obj)) {
    return create_parameter_error("First argument must be a call_context dictionary");
  }

  try {
    // 1. Create call_context (token is optional)
    CallContext ctx = CallContext::from_python_dict(call_context_obj, "IQuery_Template_echoString");

    // 2. Validate context (token validation only occurs if token is present)
    ctx.validate("IQuery_Template_echoString");

    // 3. Execute query
    Cache* repo = fromRepository();
    if (!repo) {
      throw std::runtime_error("Repository not available");
    }

    auto result = repo->IQuery_Template_echoString(input_str);

    // 4. Convert result to Python object
    if (!result.has_value()) {
      return create_success_object(nullptr);
    }

    PyObject* py_result = PyUnicode_FromString(result.value().c_str());
    if (!py_result) {
      return create_system_error("Failed to convert result to Python string");
    }

    PyObject* success_obj = create_success_object(py_result);
    Py_DECREF(py_result);
    return success_obj;

  } catch (const CallContext::AuthError& e) {
    return create_error_object("AUTH", e.what());
  } catch (const CallContext::ValidationError& e) {
    return create_error_object(e.error_type(), e.what());
  } catch (const std::exception& e) {
    return create_error_object("SYSTEM",
                               std::string("System error in IQuery_Template_echoString: ") + e.what());
  }
}

// =================================================================================================
// HELPER FUNCTIONS
// =================================================================================================

/**
 * Get build information
 */
static PyObject* get_build_info(PyObject* self, PyObject* args)
{
  (void)self;
  (void)args;

  PyObject* dict = PyDict_New();
  if (!dict) {
    return nullptr;
  }

  PyDict_SetItemString(dict, "module", PyUnicode_FromString(PYTHON_MODULE_NAME_STR));

#ifdef CAOS_PYTHON_DEBUG
  PyDict_SetItemString(dict, "debug", Py_True);
#else
  PyDict_SetItemString(dict, "debug", Py_False);
#endif

  return dict;
}

/**
 * Validate a token against a specific query (for testing)
 */
static PyObject* validate_token(PyObject* self, PyObject* args)
{
  const char* query_name;
  const char* token = nullptr;

  if (!PyArg_ParseTuple(args, "s|s", &query_name, &token)) {
    return create_parameter_error("validate_token requires query_name and optional token");
  }

  try {
    // Create call_context dictionary
    PyObject* call_context_dict = nullptr;
    if (token && strlen(token) > 0) {
      call_context_dict = Py_BuildValue("{s:s}", "token", token);
    } else {
      call_context_dict = Py_BuildValue("{}");  // Empty dict
    }

    if (!call_context_dict) {
      return create_system_error("Failed to create call_context dictionary");
    }

    CallContext ctx = CallContext::from_python_dict(call_context_dict, query_name);
    Py_DECREF(call_context_dict);

    ctx.validate(query_name);

    return create_success_object(Py_True);

  } catch (const CallContext::AuthError& e) {
    return create_error_object("AUTH", e.what());
  } catch (const CallContext::ValidationError& e) {
    return create_error_object(e.error_type(), e.what());
  } catch (const std::exception& e) {
    return create_error_object("SYSTEM", e.what());
  }
}

/**
 * Check if a query requires authentication
 */
static PyObject* query_requires_auth(PyObject* self, PyObject* args)
{
  const char* query_name;

  if (!PyArg_ParseTuple(args, "s", &query_name)) {
    return create_parameter_error("query_requires_auth requires query_name");
  }

  try {
    // Create a dummy call_context without token
    PyObject* empty_dict = Py_BuildValue("{}");
    if (!empty_dict) {
      return create_system_error("Failed to create dictionary");
    }

    CallContext ctx = CallContext::from_python_dict(empty_dict, query_name);
    Py_DECREF(empty_dict);

    // Try to validate - will only fail if query requires auth
    try {
      ctx.validate(query_name);
      // Query doesn't require auth
      return create_success_object(Py_False);
    } catch (const CallContext::AuthError&) {
      // Query requires authentication
      return create_success_object(Py_True);
    } catch (const CallContext::ValidationError& e) {
      return create_error_object(e.error_type(), e.what());
    }

  } catch (const std::exception& e) {
    return create_error_object("SYSTEM", e.what());
  }
}

/**
 * Create call_context dictionary (helper function)
 */
static PyObject* create_call_context(PyObject* self, PyObject* args)
{
  const char* token = nullptr;

  if (!PyArg_ParseTuple(args, "|s", &token)) {
    return create_parameter_error("create_call_context requires optional token string");
  }

  if (token && strlen(token) > 0) {
    // Create dict with token
    return Py_BuildValue("{s:s}", "token", token);
  } else {
    // Create empty dict (no token)
    return Py_BuildValue("{}");
  }
}

// =================================================================================================
// MODULE REGISTRATION
// =================================================================================================

/**
 * Module method table
 */
static PyMethodDef PYTHON_METHODS_TABLE[] = {
  // Query function
  {
    "IQuery_Template_echoString",
    IQuery_Template_echoString,
    METH_VARARGS,
    "Echo a string through CAOS repository\n"
    "\n"
    "Args:\n"
    "    call_context (dict): Context dictionary. Token is optional.\n"
    "    str (str): String to echo\n"
    "\n"
    "Returns:\n"
    "    dict: {'success': bool, 'data': str or None}\n"
    "           On error: {'success': False, 'error_type': str, 'error_message': str}\n"
  },

  // Helper functions
  {
    "get_build_info",
    get_build_info,
    METH_NOARGS,
    "Get build information about this module\n"
    "\n"
    "Returns:\n"
    "    dict: Module information including backend, build number, etc.\n"
  },
  {
    "validate_token",
    validate_token,
    METH_VARARGS,
    "Validate a token against a specific query\n"
    "\n"
    "Args:\n"
    "    query_name (str): Name of the query to validate against\n"
    "    token (str, optional): Token to validate.\n"
  },
  {
    "query_requires_auth",
    query_requires_auth,
    METH_VARARGS,
    "Check if a query requires authentication\n"
    "\n"
    "Args:\n"
    "    query_name (str): Name of the query to check\n"
  },
  {
    "create_call_context",
    create_call_context,
    METH_VARARGS,
    "Create a call_context dictionary\n"
    "\n"
    "Args:\n"
    "    token (str, optional): Authentication token\n"
  },

  // Sentinel
  {NULL, NULL, 0, NULL}
};

/**
 * Module definition
 */
static PyModuleDef PYTHON_MODULE_DEF = {
  PyModuleDef_HEAD_INIT,
  PYTHON_MODULE_NAME_STR,          // Module name
  CAOS_PYTHON_DESCRIPTION,         // Module documentation
  -1,                              // Module state size (-1 = global state)
  PYTHON_METHODS_TABLE,            // Method table
  NULL,                            // m_slots
  NULL,                            // m_traverse
  NULL,                            // m_clear
  NULL                             // m_free
};

/**
 * Module initialization function
 */
PyMODINIT_FUNC PYTHON_INIT_FUNCTION(void)
{
  // Initialize CAOS (lazy initialization)
  libcaos();

  // Register cleanup handler
  Py_AtExit(caos_shutdown);

  // Register default validators
  CallContext::register_validator(std::make_unique<CallContext::TokenValidator>());

  // Create the module
  PyObject* module = PyModule_Create(&PYTHON_MODULE_DEF);
  if (!module) {
    return NULL;
  }

  // Add module-level constants
  PyModule_AddStringConstant(module, "__version__", "0.1.0");

  // Add authentication information
  PyModule_AddStringConstant(module, "__auth_system__", "caosFilter::Auth::Token");
  PyModule_AddStringConstant(module, "__call_context_format__",
                             "dict with optional 'token' field");

#ifdef CAOS_PYTHON_DEBUG
  PyModule_AddIntConstant(module, "__debug__", 1);
#else
  PyModule_AddIntConstant(module, "__debug__", 0);
#endif

  return module;
}
