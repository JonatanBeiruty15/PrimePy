// primepy_binding.cpp
#include <pybind11/pybind11.h>
namespace py = pybind11;

// Forward declarations of all binding setup functions
void bind_algebra(py::module_&);



PYBIND11_MODULE(_core, m) {
    auto algebra = m.def_submodule("algebra");
    bind_algebra(algebra);
}