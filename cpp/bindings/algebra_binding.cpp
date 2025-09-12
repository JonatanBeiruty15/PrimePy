#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../include/primepy/algebra/algebraic_structures_exp.h"
#include "../include/primepy/algebra/algebraic_structures_utils.h"

namespace py = pybind11;
using namespace primepy::algebra;




void bind_algebra(py::module_& m) {
//=======================================================================================
// Groups
//=======================================================================================

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


//=======================================================================================
// Rings
//=======================================================================================

    // Bind the abstract Ring<int> so Python can hold references/polymorphism.
    // (No constructor; it's abstract. Expose useful methods.)
    py::class_<Ring<int>, Group<int>, std::shared_ptr<Ring<int>>>(m, "Ring")
        // additive part already available via Group<int> (identity/inverse/operate)
        .def("zero", &Ring<int>::zero)
        .def("one",  &Ring<int>::one)
        .def("add",  &Ring<int>::add)
        .def("neg",  &Ring<int>::neg)
        .def("mul",  &Ring<int>::mul)
        .def("is_equal", &Ring<int>::is_equal)
        // additive power for any Ring via GroupUtils ---
        .def("power", [](const Ring<int>& r, int base, int exp) {
                return GroupUtils<int>::power(r, base, exp);           // additive
            }, py::arg("base"), py::arg("exponent"),
            py::call_guard<py::gil_scoped_release>())
        .def("power", [](const Ring<int>& r, const std::vector<int>& bases, int exp) {
                return GroupUtils<int>::power(r, bases, exp);          // additive
            }, py::arg("bases"), py::arg("exponent"),
            py::call_guard<py::gil_scoped_release>())
        .def("power", [](const Ring<int>& r, const std::vector<int>& bases, const std::vector<int>& exps) {
                return GroupUtils<int>::power(r, bases, exps);         // additive
            }, py::arg("bases"), py::arg("exponents"),
            py::call_guard<py::gil_scoped_release>());

    // Ring of integers modulo n: ℤ/nℤ
    py::class_<IntegersModRing, Ring<int>, std::shared_ptr<IntegersModRing>>(m, "IntegersModRing")
        .def(py::init<int>(), py::arg("modulus"))
        // ring primitives
        .def("zero", &IntegersModRing::zero)
        .def("one",  &IntegersModRing::one)
        .def("add",  &IntegersModRing::add)
        .def("neg",  &IntegersModRing::neg)
        .def("mul",  &IntegersModRing::mul)
        // hooks/utilities
        .def("is_equal", &IntegersModRing::is_equal)
        .def("contains", &IntegersModRing::contains)
        .def_property_readonly("modulus", &IntegersModRing::modulus)
        // multiplicative power via RingUtils — use a distinct name to avoid clobbering additive Ring::power
        .def("mpower", [](const IntegersModRing& R, int base, long long exp) {
                return RingUtils<int>::power(R, base, exp);
            }, py::arg("base"), py::arg("exponent"),
            py::call_guard<py::gil_scoped_release>())
        .def("mpower", [](const IntegersModRing& R, const std::vector<int>& bases, long long exp) {
                return RingUtils<int>::power(R, bases, exp);
            }, py::arg("bases"), py::arg("exponent"),
            py::call_guard<py::gil_scoped_release>())
        .def("mpower", [](const IntegersModRing& R, const std::vector<int>& bases, const std::vector<long long>& exponents) {
                return RingUtils<int>::power(R, bases, exponents);
            }, py::arg("bases"), py::arg("exponents"),
            py::call_guard<py::gil_scoped_release>());
            
    }