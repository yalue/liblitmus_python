#define PY_SSIZE_T_CLEAN
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <Python.h>
#include <litmus.h>
#include <unistd.h>

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

static PyObject* WaitForJobRelease(PyObject *self, PyObject *args) {
  unsigned int job_number;
  int result;
  if (!PyArg_ParseTuple(args, "I", &job_number)) return NULL;
  result = wait_for_job_release(job_number);
  if (result != 0) {
    SetLITMUSError("wait_for_job_release", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* GetJobNumber(PyObject *self, PyObject *args) {
  unsigned int job_number;
  int result;
  result = get_job_no(&job_number);
  if (result != 0) {
    SetLITMUSError("get_job_no", result);
    return NULL;
  }
  return Py_BuildValue("I", job_number);
}

static PyObject* SleepNextPeriod(PyObject *self, PyObject *args) {
  int result = sleep_next_period();
  if (result != 0) {
    SetLITMUSError("sleep_next_period", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* SetRTTaskParam(PyObject *self, PyObject *args,
    PyObject *kwargs) {
  struct rt_task param;
  long long cost, period, deadline, phase;
  unsigned int cpu, priority, class, budget_policy, release_policy;
  int result;

  // These defaults are basically the same as in liblitmus/src/litmus.c. Any
  // non-provided keyword arguments will keep these default values.
  cost = 0;
  period = 0;
  deadline = 0;
  phase = 0;
  cpu = 0;
  priority = LITMUS_LOWEST_PRIORITY;
  class = RT_CLASS_SOFT;
  budget_policy = NO_ENFORCEMENT;
  release_policy = TASK_SPORADIC;

  char *kw_list[] = {
    "exec_cost",
    "period",
    "relative_deadline",
    "phase",
    "cpu",
    "priority",
    "cls",
    "budget_policy",
    "release_policy",
    NULL,
  };

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|$LLLLIIIII", kw_list, &cost,
    &period, &deadline, &phase, &cpu, &priority, &class, &budget_policy,
    &release_policy)) {
    return NULL;
  }
  if (!cost || !period) {
    PyErr_Format(PyExc_ValueError, "The exec_cost and period arguments to "
      "set_rt_task_param must be provided and nonzero.");
    return NULL;
  }
  init_rt_task_param(&param);
  param.exec_cost = cost;
  param.period = period;
  param.relative_deadline = deadline;
  param.phase = phase;
  param.cpu = cpu;
  param.priority = priority;
  param.cls = class;
  param.budget_policy = budget_policy;
  param.release_policy = release_policy;

  result = set_rt_task_param(gettid(), &param);
  if (result != 0) {
    SetLITMUSError("set_rt_task_param", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* GetRTTaskParam(PyObject *self, PyObject *args) {
  struct rt_task param;
  long long cost, period, deadline, phase;
  unsigned int cpu, priority, class, budget_policy, release_policy;
  int result;
  result = get_rt_task_param(gettid(), &param);
  if (result != 0) {
    SetLITMUSError("get_rt_task_param", result);
    return NULL;
  }
  cost = param.exec_cost;
  period = param.period;
  deadline = param.relative_deadline;
  phase = param.phase;
  cpu = param.cpu;
  priority = param.priority;
  class = param.cls;
  budget_policy = param.budget_policy;
  release_policy = param.release_policy;
  return Py_BuildValue("{s:L,s:L,s:L,s:L,s:I,s:I,s:I,s:I,s:I}",
    "exec_cost", cost, "period", period, "relative_deadline", deadline,
    "phase", phase, "cpu", cpu, "priority", priority, "cls", class,
    "budget_policy", budget_policy, "release_policy", release_policy);
}

static PyObject* GetControlPage(PyObject *self, PyObject *args) {
  unsigned long np_flag_sched, irq_count, ts_syscall_start, irq_syscall_start,
    deadline, release, job_index, k_exclusion_slot;
  struct control_page *ctrl_page = get_ctrl_page();
  if (!ctrl_page) {
    SetLITMUSError("get_ctrl_page", 0);
    return NULL;
  }

  // I assign all these to "unsigned longs" to ensure they're the correct type
  // used by Py_BuildValue.
  np_flag_sched = ctrl_page->sched.raw;
  irq_count = ctrl_page->irq_count;
  ts_syscall_start = ctrl_page->ts_syscall_start;
  irq_syscall_start = ctrl_page->irq_syscall_start;
  deadline = ctrl_page->deadline;
  release = ctrl_page->release;
  job_index = ctrl_page->job_index;
  k_exclusion_slot = ctrl_page->k_exclusion_slot;
  return Py_BuildValue("{s:k,s:k,s:k,s:k,s:k,s:k,s:k,s:k}",
    "sched", np_flag_sched,
    "irq_count", irq_count,
    "ts_syscall_start", ts_syscall_start,
    "irq_syscall_start", irq_syscall_start,
    "deadline", deadline,
    "release", release,
    "job_index", job_index,
    "k_exclusion_slot", k_exclusion_slot);
}

static PyObject* GetKExclusionSlot(PyObject *self, PyObject *args) {
  struct control_page *ctrl_page = get_ctrl_page();
  if (!ctrl_page) {
    SetLITMUSError("get_ctrl_page", 0);
    return NULL;
  }
  return Py_BuildValue("k", (unsigned long) ctrl_page->k_exclusion_slot);
}

static PyObject* LockProtocolForName(PyObject *self, PyObject *args) {
  char *s = NULL;
  int result;
  if (!PyArg_ParseTuple(args, "s", &s)) return NULL;
  result = lock_protocol_for_name(s);
  return Py_BuildValue("i", result);
}

static PyObject* NameForLockProtocol(PyObject *self, PyObject *args) {
  const char *s = NULL;
  int arg;
  if (!PyArg_ParseTuple(args, "i", &arg)) return NULL;
  s = name_for_lock_protocol(arg);
  return Py_BuildValue("s", s);
}

static PyObject* OpenKFMLPLock(PyObject *self, PyObject *args) {
  const char *namespace = NULL;
  int name, k, lock_od;
  if (!PyArg_ParseTuple(args, "sii", &namespace, &name, &k)) return NULL;
  lock_od = litmus_open_lock(K_FMLP_SEM, name, namespace, &k);
  if (lock_od < 0) {
    SetLITMUSError("litmus_open_lock", lock_od);
    return NULL;
  }
  return Py_BuildValue("i", lock_od);
}

static PyObject* CloseOD(PyObject *self, PyObject *args) {
  int arg, result;
  if (!PyArg_ParseTuple(args, "i", &arg)) return NULL;
  result = od_close(arg);
  if (result != 0) {
    SetLITMUSError("od_close", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* LITMUSLock(PyObject *self, PyObject *args) {
  int arg, result;
  if (!PyArg_ParseTuple(args, "i", &arg)) return NULL;
  result = litmus_lock(arg);
  if (result != 0) {
    SetLITMUSError("litmus_lock", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* LITMUSUnlock(PyObject *self, PyObject *args) {
  int arg, result;
  if (!PyArg_ParseTuple(args, "i", &arg)) return NULL;
  result = litmus_unlock(arg);
  if (result != 0) {
    SetLITMUSError("litmus_unlock", result);
    return NULL;
  }
  Py_RETURN_NONE;
}

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
  {
    "get_job_no",
    GetJobNumber,
    METH_NOARGS,
    "Returns the current job's number, I think. (Not documented in litmus.h)",
  },
  {
    "wait_for_job_release",
    WaitForJobRelease,
    METH_VARARGS,
    "Requires a single job number as an arg. Waits for it to be released. "
      "(I guess. Also not documented in litmus.h.)",
  },
  {
    "sleep_next_period",
    SleepNextPeriod,
    METH_NOARGS,
    "Sleep until the next period.",
  },
  {
    "set_rt_task_param",
    (PyCFunction) SetRTTaskParam,
    METH_VARARGS | METH_KEYWORDS,
    "Sets the parameters when initializing an RT task. Sets the parameters for"
      " the current thread. The exec_cost and period keyword args must be "
      "provided and nonzero. Other keyword args have the same names as the "
      "fields in the rt_task struct.",
  },
  {
    "get_rt_task_param",
    GetRTTaskParam,
    METH_NOARGS,
    "Returns a dict with fields corresponding to the values in the rt_task "
      "struct.",
  },
  {
    "get_ctrl_page",
    GetControlPage,
    METH_NOARGS,
    "Returns the contents of the current task's control page as a dict.",
  },
  {
    "get_k_exclusion_slot",
    GetKExclusionSlot,
    METH_NOARGS,
    "Like get_ctrl_page, but only returns a single int: the slot the task "
      "currently occupies if it holds a k-exclusion lock.",
  },
  {
    "lock_protocol_for_name",
    LockProtocolForName,
    METH_VARARGS,
    "Takes a name of a locking protocol and returns its semaphore type number",
  },
  {
    "name_for_lock_protocol",
    NameForLockProtocol,
    METH_VARARGS,
    "Takes a semaphore type number and returns its name as a string",
  },
  {
    "open_kfmlp_lock",
    OpenKFMLPLock,
    METH_VARARGS,
    "Requires a string name for a shared file to use, a user-chosen lock ID "
      "integer and the k-value for the lock. Returns the object descriptor "
      "(integer) for the lock.",
  },
  {
    "od_close",
    CloseOD,
    METH_VARARGS,
    "Closes an object descriptor. Requires a single int arg (the OD).",
  },
  {
    "litmus_lock",
    LITMUSLock,
    METH_VARARGS,
    "Obtains a lock. Requires a single arg: the lock's OD.",
  },
  {
    "litmus_unlock",
    LITMUSUnlock,
    METH_VARARGS,
    "Releases a lock. Requires a single arg: the lock's OD.",
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
