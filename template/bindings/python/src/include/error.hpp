#pragma once

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
  if (!dict)
  {
    return nullptr;
  }

  // success: false
  PyDict_SetItemString(dict, "success", Py_False);

  // error_type
  PyObject* error_type_str = PyUnicode_FromString(error_type.c_str());
  if (!error_type_str)
  {
    Py_DECREF(dict);
    return nullptr;
  }
  PyDict_SetItemString(dict, "error_type", error_type_str);
  Py_DECREF(error_type_str);

  // error_message
  PyObject* error_msg_str = PyUnicode_FromString(error_message.c_str());
  if (!error_msg_str)
  {
    Py_DECREF(dict);
    return nullptr;
  }
  PyDict_SetItemString(dict, "error_message", error_msg_str);
  Py_DECREF(error_msg_str);

  // data
  if (additional_data)
  {
    Py_INCREF(additional_data);
    PyDict_SetItemString(dict, "data", additional_data);
  }
  else
  {
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
  if (!dict)
  {
    return nullptr;
  }

  // success: true
  PyDict_SetItemString(dict, "success", Py_True);

  // data
  if (data)
  {
    Py_INCREF(data);
    PyDict_SetItemString(dict, "data", data);
  }
  else
  {
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
