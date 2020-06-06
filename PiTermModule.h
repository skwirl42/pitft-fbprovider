#ifndef _PITERM_MODULE_H_
#define _PITERM_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PY_SSIZE_T_CLEAN
#include <Python.h>

PyObject* PyInit_PiTerm();

#ifdef __cplusplus
}
#endif

#endif // _PITERM_MODULE_H_
