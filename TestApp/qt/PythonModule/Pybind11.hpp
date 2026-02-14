#ifndef GP_PYTHON_UTILS_H
#define GP_PYTHON_UTILS_H

// 1. Save the state of the keywords and undefine them
#pragma push_macro("slots")
#pragma push_macro("signals")
#undef slots
#undef signals

#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#pragma pop_macro("slots")
#pragma pop_macro("signals")

#endif