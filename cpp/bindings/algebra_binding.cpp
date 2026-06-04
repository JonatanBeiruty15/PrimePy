#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../include/primepy/algebra/algebraic_structures_exp.h"
#include "../include/primepy/algebra/algebraic_structures_utils.h"

namespace py = pybind11;
using namespace primepy::algebra;

namespace {

Integer integer_from_py(py::handle raw) {
    if (!py::isinstance<py::int_>(raw)) {
        throw std::invalid_argument("expected an integer.");
    }
    return Integer(py::str(raw).cast<std::string>());
}

Rational rational_from_py(py::handle raw) {
    if (py::isinstance<py::int_>(raw)) {
        return Rational(integer_from_py(raw));
    }
    if (py::isinstance<py::float_>(raw)) {
        py::object fraction = py::module_::import("fractions").attr("Fraction")(
            py::str(py::repr(raw))
        );
        Integer numerator(py::str(fraction.attr("numerator")).cast<std::string>());
        Integer denominator(py::str(fraction.attr("denominator")).cast<std::string>());
        Rational value(numerator, denominator);
        value.canonicalize();
        return value;
    }
    if (!py::hasattr(raw, "numerator") || !py::hasattr(raw, "denominator")) {
        throw std::invalid_argument("expected an integer, float, or fractions.Fraction.");
    }

    Integer numerator(py::str(raw.attr("numerator")).cast<std::string>());
    Integer denominator(py::str(raw.attr("denominator")).cast<std::string>());
    if (denominator == 0) {
        throw std::invalid_argument("rational conversion: denominator cannot be zero.");
    }

    Rational value(numerator, denominator);
    value.canonicalize();
    return value;
}

bool is_rational_like(py::handle raw) {
    return py::isinstance<py::float_>(raw)
        || (py::hasattr(raw, "numerator") && py::hasattr(raw, "denominator"));
}

py::object integer_to_py(const Integer& value) {
    return py::module_::import("builtins").attr("int")(value.get_str());
}

py::object rational_to_py(const Rational& value) {
    return py::module_::import("fractions").attr("Fraction")(
        integer_to_py(value.get_num()),
        integer_to_py(value.get_den())
    );
}

py::object element_value_to_py(const Element& element) {
    if (std::holds_alternative<Integer>(element.data())) {
        return integer_to_py(std::get<Integer>(element.data()));
    }

    if (std::holds_alternative<Rational>(element.data())) {
        return rational_to_py(std::get<Rational>(element.data()));
    }

    const auto& parts = std::get<Element::Vector>(element.data());
    py::tuple out(parts.size());
    for (std::size_t i = 0; i < parts.size(); ++i) {
        out[i] = element_value_to_py(parts[i]);
    }
    return std::move(out);
}

Element element_from_py(std::shared_ptr<const Group> group, py::handle raw) {
    if (!group) {
        throw std::invalid_argument("element conversion: group must be non-null.");
    }

    if (py::isinstance<Element>(raw)) {
        Element element = raw.cast<Element>();
        if (!group->contains(element)) {
            throw std::invalid_argument("element conversion: element belongs to a different group.");
        }
        return element;
    }

    if (py::isinstance<py::int_>(raw)) {
        Integer value = integer_from_py(raw);
        auto polynomial_ring = std::dynamic_pointer_cast<const PolynomialRing>(group);
        if (polynomial_ring) {
            return polynomial_ring->coefficient_ring()->element(value);
        }
        return group->element(value);
    }

    if (is_rational_like(raw)) {
        Rational value = rational_from_py(raw);
        auto polynomial_ring = std::dynamic_pointer_cast<const PolynomialRing>(group);
        if (polynomial_ring) {
            return polynomial_ring->coefficient_ring()->element(value);
        }
        return group->element(value);
    }

    if (py::isinstance<py::tuple>(raw) || py::isinstance<py::list>(raw)) {
        auto direct_sum = std::dynamic_pointer_cast<const DirectSumGroup>(group);
        auto polynomial_ring = std::dynamic_pointer_cast<const PolynomialRing>(group);

        if (!direct_sum && !polynomial_ring) {
            throw std::invalid_argument(
                "element conversion: only direct sums and polynomial rings accept tuple/list elements.");
        }

        py::sequence seq = py::reinterpret_borrow<py::sequence>(raw);
        if (direct_sum && seq.size() != static_cast<py::ssize_t>(direct_sum->arity())) {
            throw std::invalid_argument(
                "element conversion: expected tuple/list of length " +
                std::to_string(direct_sum->arity()) +
                ", got length " + std::to_string(seq.size()) + ".");
        }

        Element::Vector parts;
        parts.reserve(static_cast<std::size_t>(seq.size()));

        if (direct_sum) {
            for (py::ssize_t i = 0; i < seq.size(); ++i) {
                parts.push_back(element_from_py(direct_sum->factor(static_cast<std::size_t>(i)), seq[i]));
            }
            return direct_sum->element(parts);
        }

        for (py::ssize_t i = 0; i < seq.size(); ++i) {
            parts.push_back(element_from_py(polynomial_ring->coefficient_ring(), seq[i]));
        }
        return polynomial_ring->element(parts);
    }

    throw std::invalid_argument("element conversion: expected int, tuple/list, or Element.");
}

std::vector<Element> elements_from_py_sequence(std::shared_ptr<const Group> group, py::handle raw) {
    if (!(py::isinstance<py::tuple>(raw) || py::isinstance<py::list>(raw))) {
        throw std::invalid_argument("expected a list/tuple of group elements.");
    }

    py::sequence seq = py::reinterpret_borrow<py::sequence>(raw);
    std::vector<Element> out;
    out.reserve(static_cast<std::size_t>(seq.size()));
    for (py::ssize_t i = 0; i < seq.size(); ++i) {
        out.push_back(element_from_py(group, seq[i]));
    }
    return out;
}

std::vector<Integer> integers_from_py_sequence(py::handle raw) {
    if (!(py::isinstance<py::tuple>(raw) || py::isinstance<py::list>(raw))) {
        throw std::invalid_argument("expected a list/tuple of integers.");
    }

    py::sequence seq = py::reinterpret_borrow<py::sequence>(raw);
    std::vector<Integer> out;
    out.reserve(static_cast<std::size_t>(seq.size()));
    for (py::ssize_t i = 0; i < seq.size(); ++i) {
        out.push_back(integer_from_py(seq[i]));
    }
    return out;
}

} // namespace




void bind_algebra(py::module_& m) {
//=======================================================================================
// Groups
//=======================================================================================

    py::enum_<GroupProperty>(m, "GroupProperty")
        .value("Finite", GroupProperty::Finite)
        .value("Abelian", GroupProperty::Abelian)
        .export_values();

    py::class_<Element>(m, "Element")
        .def_property_readonly("value", [](const Element& element) {
            return element_value_to_py(element);
        })
        .def("__repr__", [](const Element& element) {
            return element.repr();
        })
        .def("__eq__", [](const Element& a, const Element& b) {
            return a.equals(b);
        });

    py::class_<Group, std::shared_ptr<Group>>(m, "Group")
        .def_property_readonly("properties", &Group::properties)
        .def("has_property", &Group::has_property, py::arg("property"))
        .def("element", [](std::shared_ptr<Group> g, py::object raw) {
            return element_from_py(g, raw);
        }, py::arg("value"))
        .def("__call__", [](std::shared_ptr<Group> g, py::object raw) {
            return element_from_py(g, raw);
        }, py::arg("value"))
        .def("identity", &Group::identity)
        .def("inverse", [](std::shared_ptr<Group> g, py::object raw) {
            return g->inverse(element_from_py(g, raw));
        }, py::arg("a"))
        .def("operate", [](std::shared_ptr<Group> g, py::object a, py::object b) {
            return g->operate(element_from_py(g, a), element_from_py(g, b));
        }, py::arg("a"), py::arg("b"))
        .def("contains", [](std::shared_ptr<Group> g, py::object raw) {
            try {
                return g->contains(element_from_py(g, raw));
            } catch (...) {
                return false;
            }
        }, py::arg("a"))
        .def("power", [](std::shared_ptr<Group> g, py::object base, long long exp) {
            return g->power(element_from_py(g, base), exp);
        }, py::arg("base"), py::arg("exponent"))
        .def("power_many", [](std::shared_ptr<Group> g, py::object bases, long long exp) {
            return g->power(elements_from_py_sequence(g, bases), exp);
        }, py::arg("bases"), py::arg("exponent"))
        .def("power_many", [](std::shared_ptr<Group> g, py::object bases, const std::vector<long long>& exps) {
            return g->power(elements_from_py_sequence(g, bases), exps);
        }, py::arg("bases"), py::arg("exponents"));
//=============================
// Additive Group mod n: ℤ/nℤ
//=============================
    py::class_<AdditiveModGroup, Group, std::shared_ptr<AdditiveModGroup>>(m, "AdditiveModGroup")
        .def(py::init<int>())
        .def_property_readonly("modulus", &AdditiveModGroup::modulus);
// =============================
// Multiplicative Group mod n (ℤ/nℤ)^*
// =============================
    py::class_<MultiplicativeModGroup, Group, std::shared_ptr<MultiplicativeModGroup>>(m, "MultiplicativeModGroup")
        .def(py::init<int>(), py::arg("modulus"))
        .def_property_readonly("modulus", &MultiplicativeModGroup::modulus);

//=============================
// Direct sum construction for groups
//=============================
    py::class_<DirectSumGroup, Group, std::shared_ptr<DirectSumGroup>>(m, "DirectSumGroup")
        .def(py::init([](const std::vector<std::shared_ptr<Group>>& factors) {
            std::vector<std::shared_ptr<const Group>> const_factors;
            const_factors.reserve(factors.size());
            for (const auto& factor : factors) {
                const_factors.push_back(factor);
            }
            return std::make_shared<DirectSumGroup>(std::move(const_factors));
        }), py::arg("factors"))
        .def_property_readonly("arity", &DirectSumGroup::arity);

    m.def("direct_sum",
          [](const std::vector<std::shared_ptr<Group>>& factors) {
              std::vector<std::shared_ptr<const Group>> const_factors;
              const_factors.reserve(factors.size());
              for (const auto& factor : factors) {
                  const_factors.push_back(factor);
              }
              return GroupUtils::direct_sum(std::move(const_factors));
          },
          py::arg("factors"),
          "Build a direct sum of runtime groups.");






//=======================================================================================
// Rings
//=======================================================================================

    py::class_<Ring, Group, std::shared_ptr<Ring>>(m, "Ring")
        .def("zero", &Ring::zero)
        .def("one", &Ring::one)
        .def("add", [](std::shared_ptr<Ring> r, py::object a, py::object b) {
            return r->add(element_from_py(r, a), element_from_py(r, b));
        }, py::arg("a"), py::arg("b"))
        .def("neg", [](std::shared_ptr<Ring> r, py::object a) {
            return r->neg(element_from_py(r, a));
        }, py::arg("a"))
        .def("mul", [](std::shared_ptr<Ring> r, py::object a, py::object b) {
            return r->mul(element_from_py(r, a), element_from_py(r, b));
        }, py::arg("a"), py::arg("b"))
        .def("is_equal", [](std::shared_ptr<Ring> r, py::object a, py::object b) {
            return r->equals(element_from_py(r, a), element_from_py(r, b));
        }, py::arg("a"), py::arg("b"))
        .def("mpower", [](std::shared_ptr<Ring> r, py::object base, long long exp) {
            return r->mpower(element_from_py(r, base), exp);
        }, py::arg("base"), py::arg("exponent"))
        .def("mpower_many", [](std::shared_ptr<Ring> r, py::object bases, long long exp) {
            return r->mpower(elements_from_py_sequence(r, bases), exp);
        }, py::arg("bases"), py::arg("exponent"))
        .def("mpower_many", [](std::shared_ptr<Ring> r, py::object bases, const std::vector<long long>& exps) {
            return r->mpower(elements_from_py_sequence(r, bases), exps);
        }, py::arg("bases"), py::arg("exponents"));

    py::class_<Field, Ring, std::shared_ptr<Field>>(m, "Field")
        .def("reciprocal", [](std::shared_ptr<Field> f, py::object a) {
            return f->reciprocal(element_from_py(f, a));
        }, py::arg("a"))
        .def("div", [](std::shared_ptr<Field> f, py::object a, py::object b) {
            return f->div(element_from_py(f, a), element_from_py(f, b));
        }, py::arg("a"), py::arg("b"))
        .def_property_readonly("characteristic", [](const Field& f) {
            return integer_to_py(f.characteristic());
        });

    // Ring of integers modulo n: ℤ/nℤ
    py::class_<IntegersModRing, Ring, std::shared_ptr<IntegersModRing>>(m, "IntegersModRing")
        .def(py::init<int>(), py::arg("modulus"))
        .def_property_readonly("modulus", &IntegersModRing::modulus);

    py::class_<RationalField, Field, std::shared_ptr<RationalField>>(m, "RationalField")
        .def(py::init<>());

    py::class_<FiniteField, Field, std::shared_ptr<FiniteField>>(m, "FiniteField")
        .def(py::init<int, int>(), py::arg("prime"), py::arg("degree") = 1)
        .def_property_readonly("modulus", &FiniteField::modulus)
        .def_property_readonly("degree", &FiniteField::degree);

    // Polynomial ring in one variable: R[X]
    py::class_<PolynomialRing, Ring, std::shared_ptr<PolynomialRing>>(m, "PolynomialRing")
        .def(py::init([](std::shared_ptr<Ring> coefficient_ring,
                         const std::string& variable,
                         bool use_parentheses) {
            if (variable.size() != 1) {
                throw std::invalid_argument("PolynomialRing: variable must be exactly one character.");
            }
            return std::make_shared<PolynomialRing>(
                std::static_pointer_cast<const Ring>(coefficient_ring),
                variable[0],
                use_parentheses
            );
        }),
        py::arg("coefficient_ring"),
        py::arg("variable") = "X",
        py::arg("use_parentheses") = true)
        .def_property_readonly("variable", [](const PolynomialRing& ring) {
            return std::string(1, ring.variable());
        });
        
            
//#####################################
// Ring of integers: ℤ  
//#####################################
    py::class_<Integers, Ring, std::shared_ptr<Integers>>(m, "Integers")
        .def(py::init<>())  // trivial ctor
        // static helpers
        .def_static("gcd", &Integers::gcd, py::arg("a"), py::arg("b"))

        // deterministic primality (scalar + batch)
        .def_static("is_prime",
            [](py::object n) {
                Integer value = integer_from_py(n);
                py::gil_scoped_release release;
                return Integers::is_prime(value);
            },
            py::arg("n"))
        .def_static("is_prime_u64",
            &Integers::is_prime_u64,
            py::arg("n"),
            py::call_guard<py::gil_scoped_release>())
        .def_static("is_prime_array",
            [](py::object numbers) {
                auto nums = integers_from_py_sequence(numbers);
                py::gil_scoped_release release;
                return Integers::is_prime_array(nums);
            },
            py::arg("numbers"));

    }
