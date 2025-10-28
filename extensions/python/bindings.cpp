#include <Python.h>
#include <libcaos.hpp>
#include <memory>
#include <vector>

// Stesso singleton di CAOS (puoi riutilizzare lo stesso codice)
std::unique_ptr<Caos>& libcaos()
{
  static std::unique_ptr<Caos> caos = nullptr;

  if (!caos)
  {
    std::vector<std::string> args = {"caos_python"};

    if (const char* config_path = std::getenv("CAOS_CONFIG"))
    {
      args.push_back("--config");
      args.push_back(config_path);
    }

    std::vector<char*> argv;
    for (auto& arg : args) argv.push_back(&arg[0]);

    caos = std::make_unique<Caos>(argv.size(), argv.data(), initFlags::Repository);
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
static void caos_shutdown()
{
  try
  {
    auto& caos = libcaos();
    if (caos)
    {
      caos.reset();
      // std::cout << "CAOS Python extension shutdown completed" << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "CAOS shutdown failed: " << e.what() << std::endl;
  }
}

// -------------------------------------------------------------------------------------------------
// caos_hello()
// -------------------------------------------------------------------------------------------------
static PyObject* caos_hello(PyObject* self, PyObject* args)
{
  return PyUnicode_FromString("Hello from CAOS!\n");
}

// -------------------------------------------------------------------------------------------------
// caos_echoString()
// -------------------------------------------------------------------------------------------------
static PyObject* caos_echoString(PyObject* self, PyObject* args)
{
  const char* input;

  // Parse "s" = string
  if (!PyArg_ParseTuple(args, "s", &input))
  {
    return NULL;
  }

  try
  {
    Cache* repo = fromRepository();
    if (!repo)
    {
      PyErr_SetString(PyExc_RuntimeError, "CAOS repository not available");
      return NULL;
    }

    auto repo_result = repo->IQuery_Test_echoString(input);
    if (!repo_result.has_value())
    {
      return PyUnicode_FromString("No result from repository!\n");
    }

    std::string final_result = repo_result.value() + " through Python!\n";
    return PyUnicode_FromString(final_result.c_str());

  }
  catch (const std::exception& e)
  {
    PyErr_SetString(PyExc_RuntimeError, ("CAOS error: " + std::string(e.what())).c_str());
    return NULL;
  }
}

static PyMethodDef CaosMethods[] = {
  {
    "hello",
    caos_hello,
    METH_VARARGS,
    "Return a hello message from CAOS"
  },
  {
    "echoString",
    caos_echoString,
    METH_VARARGS,
    "Echo a string through CAOS repository"
  },
  {NULL, NULL, 0, NULL}  // Sentinel
};

static struct PyModuleDef caosmodule = {
  PyModuleDef_HEAD_INIT,
  "caos_native",
  "Native CAOS bindings for Python",
  -1,
  CaosMethods
};


PyMODINIT_FUNC PyInit_caos_native(void)
{
  fromRepository();

  Py_AtExit(caos_shutdown);

  return PyModule_Create(&caosmodule);
}
