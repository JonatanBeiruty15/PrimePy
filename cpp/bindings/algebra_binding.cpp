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
//=============================
// Additive Group mod n: ℤ/nℤ
//=============================
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
// =============================
// Multiplicative Group mod n (ℤ/nℤ)^*
// =============================
    py::class_<MultiplicativeModGroup, Group<int>, std::shared_ptr<MultiplicativeModGroup>>(m, "MultiplicativeModGroup")
        .def(py::init<int>(), py::arg("modulus"))
        .def("identity", &MultiplicativeModGroup::identity)
        .def("inverse",  &MultiplicativeModGroup::inverse)
        .def("operate",  &MultiplicativeModGroup::operate)
        .def("contains", &MultiplicativeModGroup::contains)
        // Group-style power (uses the multiplicative law and inverse for negative exponents)
        .def("power",
            [](const MultiplicativeModGroup& g, int base, int exp) {
                return GroupUtils<int>::power(g, base, exp);
            },
            py::arg("base"), py::arg("exponent"),
            py::call_guard<py::gil_scoped_release>())
        .def("power",
            [](const MultiplicativeModGroup& g, const std::vector<int>& bases, int exp) {
                return GroupUtils<int>::power(g, bases, exp);
            },
            py::arg("bases"), py::arg("exponent"),
            py::call_guard<py::gil_scoped_release>())
        .def("power",
            [](const MultiplicativeModGroup& g, const std::vector<int>& bases, const std::vector<int>& exps) {
                return GroupUtils<int>::power(g, bases, exps);
            },
            py::arg("bases"), py::arg("exponents"),
            py::call_guard<py::gil_scoped_release>()); 

//=============================
// Direct product factory for groups
//=============================

    // Bind Group<std::pair<int,int>> so Python can hold product groups.
    py::class_<Group<std::pair<int,int>>, std::shared_ptr<Group<std::pair<int,int>>>>(m, "GroupPair");

    // Expose a free function to build direct products
    m.def("direct_product",
          [](std::shared_ptr<Group<int>> G1, std::shared_ptr<Group<int>> G2) {
              // delegate to GroupUtils<int>::direct_product
              return GroupUtils<int>::direct_product<int,int>(std::move(G1), std::move(G2));
          },
          py::arg("G1"), py::arg("G2"),
          "Build the direct product G1 × G2 as a new group (over pairs).");






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
        
            
//#####################################
// Ring of integers: ℤ  
//#####################################
    py::class_<Integers, Ring<int>, std::shared_ptr<Integers>>(m, "Integers")
        .def(py::init<>())  // trivial ctor

        // ring primitives
        .def("zero", &Integers::zero)
        .def("one",  &Integers::one)
        .def("add",  &Integers::add)
        .def("neg",  &Integers::neg)
        .def("mul",  &Integers::mul)

        // utilities/hooks
        .def("is_equal", &Integers::is_equal)
        .def("contains", &Integers::contains)

        // static helpers
        .def_static("gcd", &Integers::gcd, py::arg("a"), py::arg("b"))

        // deterministic primality (scalar + batch)
        .def_static("is_prime",
            &Integers::is_prime,
            py::arg("n"),
            py::call_guard<py::gil_scoped_release>())
        .def_static("is_prime_u64",
            &Integers::is_prime_u64,
            py::arg("n"),
            py::call_guard<py::gil_scoped_release>())
        .def_static("is_prime_array",
            &Integers::is_prime_array,
            py::arg("numbers"),
            py::call_guard<py::gil_scoped_release>());



//#####################################
// Ring of Polinomials over Z : ℤ[X] 
//#####################################

    py::class_<PolynomialsOverIntegers>(m, "PolynomialsOverIntegers")
        .def(py::init<>())

        // Ring primitives
        .def("zero", &PolynomialsOverIntegers::zero,
             "Return the zero polynomial [] (canonical empty form).")
        .def("one",  &PolynomialsOverIntegers::one,
             "Return the constant one polynomial [1].")

        // Additive ops
        .def("add", &PolynomialsOverIntegers::add, py::arg("f"), py::arg("g"),
             "f + g (coefficient-wise over Z).")
        .def("neg", &PolynomialsOverIntegers::neg, py::arg("f"),
             "-f")
        .def("sub", &PolynomialsOverIntegers::sub, py::arg("f"), py::arg("g"),
             "f - g")

        // Multiplication
        .def("mul", &PolynomialsOverIntegers::mul, py::arg("f"), py::arg("g"),
             "f * g using the class's default algorithm (currently naive).")
        .def("mul_naive", &PolynomialsOverIntegers::mul_naive, py::arg("f"), py::arg("g"),
             "f * g using schoolbook O(n*m).")
        .def("mul_karatsuba", &PolynomialsOverIntegers::mul_karatsuba, py::arg("f"), py::arg("g"),
             "f * g using Karatsuba (faster for larger degree).")

        // Equality / membership
        .def("is_equal",   &PolynomialsOverIntegers::is_equal,   py::arg("f"), py::arg("g"),
             "Compare canonical forms (after trimming).")
        .def("contains",   &PolynomialsOverIntegers::contains,   py::arg("f"),
             "Return True (any list[int] is valid).")

        // Static helpers
        .def_static("degree",     &PolynomialsOverIntegers::degree,     py::arg("f"),
             "Degree of f, or -1 for the zero polynomial.")
        .def_static("is_zero",    &PolynomialsOverIntegers::is_zero,    py::arg("f"),
             "Whether f is the zero polynomial (after trimming).")
        // expose a copy-normalizing helper because C++ normalize(Poly&) mutates:
        .def_static("normalized",
            [](Poly f){ PolynomialsOverIntegers::normalize(f); return f; }, py::arg("f"),
            "Return a trimmed (canonical) copy of f.")
        .def_static("to_string",  &PolynomialsOverIntegers::to_string,  py::arg("f"),
             "Pretty print like '3 + 2x^2 - x^5'.");



    }