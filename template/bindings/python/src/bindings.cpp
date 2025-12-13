/**
 * CAOS Python Extension
 *
 * Python binding for CAOS library with integrated call_context validation system.
 * All query functions require a call_context dictionary as first parameter.
 * Token is optional - validation only occurs if token is present.
 */

#include <memory>
#include <vector>
#include <iostream>
#include <string>

#include "extension_config.h"  // Generated configuration
#include "call_context.hpp"    // Call context management
#include "caosdba.hpp"
#include "error.hpp"

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
  if (!PyArg_ParseTuple(args, "Os", &call_context_obj, &input_str))
  {
    return create_parameter_error("Invalid parameters for IQuery_Template_echoString");
  }

  // Verify call_context is a dictionary
  if (!PyDict_Check(call_context_obj))
  {
    return create_parameter_error("First argument must be a call_context dictionary");
  }

  try
  {
    // 1. Create call_context (token is optional)
    CallContext ctx = CallContext::from_python_dict(call_context_obj, "IQuery_Template_echoString");

    // 2. Validate context (token validation only occurs if token is present)
    ctx.validate("IQuery_Template_echoString");

    // 3. Execute query
    Cache* repo = Repository();
    if (!repo)
    {
      throw std::runtime_error("Repository not available");
    }

    auto result = repo->IQuery_Template_echoString(input_str);

    // 4. Convert result to Python object
    if (!result.has_value())
    {
      return create_success_object(nullptr);
    }

    PyObject* py_result = PyUnicode_FromString(result.value().c_str());
    if (!py_result)
    {
      return create_system_error("Failed to convert result to Python string");
    }

    PyObject* success_obj = create_success_object(py_result);
    Py_DECREF(py_result);
    return success_obj;

  }
  catch (const CallContext::AuthError& e)
  {
    return create_error_object("AUTH", e.what());
  }
  catch (const CallContext::ValidationError& e)
  {
    return create_error_object(e.error_type(), e.what());
  }
  catch (const std::exception& e)
  {
    return create_error_object("SYSTEM", std::string("System error in IQuery_Template_echoString: ") + e.what());
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
  if (!dict)
  {
    return nullptr;
  }

  std::string module_info = std::string(PYTHON_MODULE_NAME_STR)
                            + " ["
                            + get_build_info_string()
                            + "]";
  PyDict_SetItemString(dict, "module", PyUnicode_FromString(module_info.c_str()));

#ifdef CAOS_PYTHON_DEBUG
  PyDict_SetItemString(dict, "debug", Py_True);
#else
  PyDict_SetItemString(dict, "debug", Py_False);
#endif

  return dict;
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
    "    call_context (dict): Context dictionary. Token is mandatory.\n"
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
