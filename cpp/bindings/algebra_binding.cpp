#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../include/primepy/algebra/algebraic_structures_exp.h"
#include "../include/primepy/algebra/algebraic_structures_utils.h"

namespace py = pybind11;
using namespace primepy::algebra;

void bind_algebra(py::module_& m) {
    py::class_<Group<int>, std::shared_ptr<Group<int>>>(m, "Group");

    py::class_<AdditiveModGroup, Group<int>, std::shared_ptr<AdditiveModGroup>>(m, "AdditiveModGroup")
        .def(py::init<int>())
        .def("identity", &AdditiveModGroup::identity)
        .def("inverse", &AdditiveModGroup::inverse)
        .def("operate", &AdditiveModGroup::operate)
        .def("power", [](const AdditiveModGroup& g, int base, int exp) {
            return GroupUtils<int>::power(g, base, exp);
        })
        .def("power", [](const AdditiveModGroup& g, const std::vector<int>& bases, int exp) {
            return GroupUtils<int>::power(g, bases, exp);
        })
        .def("power", [](const AdditiveModGroup& g, const std::vector<int>& bases, const std::vector<int>& exponents) {
            return GroupUtils<int>::power(g, bases, exponents);
        });
}