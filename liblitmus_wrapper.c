#define PY_SSIZE_T_CLEAN
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <Python.h>
#include <litmus.h>

static void SetLITMUSError(const char *fn, int result) {
  PyErr_Format(PyExc_OSError, "%s returned %d. Errno: %s", result,
    strerror(errno));
}

static PyObject* InitLITMUS(PyObject *self, PyObject *args) {
  int result = init_litmus();
  if (result != 0) {
    SetLITMUSError("init_litmus", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* ExitLITMUS(PyObject *self, PyObject *args) {
  exit_litmus();
  Py_RETURN_NONE;
}

static PyObject* SetTaskMode(PyObject *self, PyObject *args) {
  int arg, new_task_mode, result;
  new_task_mode = BACKGROUND_TASK;
  if (!PyArg_ParseTuple(args, "p", &arg)) return NULL;
  // True = we're an RT task.
  if (arg) {
    new_task_mode = LITMUS_RT_TASK;
  }
  result = task_mode(new_task_mode);
  if (result != 0) {
    SetLITMUSError("task_mode", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

// TODO (next): Continue implementing functions from litmus.h
static PyMethodDef liblitmus_helper_methods[] = {
  {
    "init_litmus",
    InitLITMUS,
    METH_NOARGS,
    "Initializes real-time properties for the entire program. No args.",
  },
  {
    "exit_litmus",
    ExitLITMUS,
    METH_NOARGS,
    "Cleans up real-time properties for the entire program.",
  },
  {
    "task_mode",
    SetTaskMode,
    METH_VARARGS,
    "Sets the task mode for the current thread. Takes one arg: a bool set to "
      "true if the thread should be a LITMUS_RT_TASK",
  },
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef liblitmus_helper_module = {
  PyModuleDef_HEAD_INIT,
  "liblitmus_helper",
  "A python wrapper around the liblitmus API.",
  -1,
  liblitmus_helper_methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyMODINIT_FUNC PyInit_liblitmus_helper(void) {
  return PyModule_Create(&liblitmus_helper_module);
}
