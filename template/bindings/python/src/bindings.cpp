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
// LAZY INITIALIZATION HELPER
// =================================================================================================

/**
 * RAII-style lazy initializer for CAOS library
 * Ensures CAOS is initialized only when needed and cleaned up properly
 */
class CaosLazyInitializer
{
  private:
    static bool initialized_;

    static void initialize_if_needed()
    {
      if (!initialized_)
      {
        libcaos();
        initialized_ = true;
      }
    }

  public:
    /**
     * Ensure CAOS library is initialized
     * Safe to call multiple times - initialization happens only once
     */
    static void ensure_initialized()
    {
      initialize_if_needed();
    }

    /**
     * Check if CAOS library has been initialized
     * @return true if CAOS is initialized
     */
    static bool is_initialized()
    {
      return initialized_;
    }

    /**
     * Clean up CAOS library resources
     * Safe to call even if not initialized
     */
    static void cleanup()
    {
      if (initialized_)
      {
        caos_shutdown();
        initialized_ = false;
      }
    }
};

// Static member initialization
bool CaosLazyInitializer::initialized_ = false;

// =================================================================================================
// QUERY IMPLEMENTATION - IQuery_Template_echoString
// =================================================================================================

/**
 * IQuery_Template_echoString - Echo string through CAOS repository
 * Requires call_context with optional token validation
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
    // Initialize CAOS only when needed (lazy initialization)
    CaosLazyInitializer::ensure_initialized();

    // 1. Create call_context (token is optional)
    CallContext ctx = CallContext::from_python_dict(call_context_obj, "IQuery_Template_echoString");

    // 2. Validate context (token validation only occurs if token is present)
    ctx.apply_auth_filters("IQuery_Template_echoString");

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
 * Get build information about the module
 * Does not initialize CAOS library
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

  // Add CAOS initialization state for debugging purposes
  PyDict_SetItemString(dict, "caos_initialized",
                       CaosLazyInitializer::is_initialized() ? Py_True : Py_False);

  return dict;
}

// =================================================================================================
// MODULE REGISTRATION
// =================================================================================================

/**
 * Module method table
 */
static PyMethodDef PYTHON_METHODS_TABLE[] = {
  // Query function - requires CAOS initialization
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

  // Helper functions - does not require CAOS initialization
  {
    "get_build_info",
    get_build_info,
    METH_NOARGS,
    "Get build information about this module\n"
    "\n"
    "Returns:\n"
    "    dict: Module information including backend, build number, etc.\n"
    "           caos_initialized: Boolean indicating if CAOS library is initialized\n"
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
 * Note: CAOS library is NOT initialized here for lazy loading
 */
PyMODINIT_FUNC PYTHON_INIT_FUNCTION(void)
{
  // Register cleanup handler for CAOS library
  // This ensures CAOS resources are cleaned up when Python exits
  Py_AtExit([]() {
    CaosLazyInitializer::cleanup();
  });


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
