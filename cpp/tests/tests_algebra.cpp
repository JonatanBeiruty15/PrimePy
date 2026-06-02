#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../include/primepy/algebra/algebraic_structures_exp.h"
#include "../include/primepy/algebra/algebraic_structures_utils.h"

using namespace primepy::algebra;

static std::string debug_string(const Integer& value) {
    return value.get_str();
}

template<typename T>
static std::string debug_string(const T& value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

#define EXPECT_TRUE(value) \
    if (!(value)) { \
        std::cerr << "Test failed at " << __FILE__ << ":" << __LINE__ \
                  << "\nExpected true." << std::endl; \
        return 1; \
    }

#define EXPECT_FALSE(value) \
    if ((value)) { \
        std::cerr << "Test failed at " << __FILE__ << ":" << __LINE__ \
                  << "\nExpected false." << std::endl; \
        return 1; \
    }

#define EXPECT_EQ(value, expected) \
    if ((value) != (expected)) { \
        std::cerr << "Test failed at " << __FILE__ << ":" << __LINE__ \
                  << "\nExpected: " << debug_string(expected) \
                  << ", got: " << debug_string(value) << std::endl; \
        return 1; \
    }

template<typename F>
int expect_throws(F&& f, const std::string& label) {
    try {
        f();
    } catch (const std::exception&) {
        return 0;
    }

    std::cerr << "Expected exception not thrown: " << label << std::endl;
    return 1;
}

template<typename T>
bool vectors_equal(const std::vector<T>& a, const std::vector<T>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

#define EXPECT_VEC_EQ(value, expected) \
    if (!vectors_equal((value), (expected))) { \
        std::cerr << "Vector test failed at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return 1; \
    }

static Integer raw_int(const Element& element) {
    return std::get<Integer>(element.data());
}

static const Element::Vector& raw_vector(const Element& element) {
    return std::get<Element::Vector>(element.data());
}

int main() {
    // ---------------------------------------------------------------------
    // Runtime additive group Z/5Z
    //
    // This section checks the new runtime Group/Element API for an integer-
    // backed group. Every operation returns an Element, so the tests unwrap
    // the raw integer data only for assertions.
    // ---------------------------------------------------------------------
    auto Z5 = std::make_shared<AdditiveModGroup>(5);
    auto Z7 = std::make_shared<AdditiveModGroup>(7);

    // Basic group law:
    //   identity = 0
    //   element(8) normalizes to 3 mod 5
    //   inverse(3) = 2 because 3 + 2 = 0 mod 5
    //   operate(3, 4) = 2 because 3 + 4 = 7 = 2 mod 5
    EXPECT_EQ(raw_int(Z5->identity()), 0);
    EXPECT_EQ(raw_int(Z5->element(8)), 3);
    EXPECT_EQ(raw_int(Z5->inverse(Z5->element(3))), 2);
    EXPECT_EQ(raw_int(Z5->operate(Z5->element(3), Z5->element(4))), 2);
    EXPECT_TRUE(Z5->element(3).equals(Z5->element(8)));
    EXPECT_EQ(Z5->element(8).repr(), std::string("3 mod 5"));
    EXPECT_TRUE(Z5->has_property(GroupProperty::Finite));
    EXPECT_TRUE(Z5->has_property(GroupProperty::Abelian));

    // In an additive group, "power" means repeated addition:
    //   3^4_add = 3 + 3 + 3 + 3 = 12 = 2 mod 5
    // Negative powers use the inverse:
    //   3^-1_add = -3 = 2 mod 5
    EXPECT_EQ(raw_int(Z5->power(Z5->element(3), 4)), 2);
    EXPECT_EQ(raw_int(Z5->power(Z5->element(3), -1)), 2);

    {
        // Batch power with one common exponent.
        std::vector<Element> bases{Z5->element(1), Z5->element(2), Z5->element(3)};
        auto out = Z5->power(bases, 2);
        EXPECT_EQ(raw_int(out[0]), 2);
        EXPECT_EQ(raw_int(out[1]), 4);
        EXPECT_EQ(raw_int(out[2]), 1);

        // Batch power with one exponent per base. Exponent 0 must return the
        // group identity, not the original base.
        auto elemwise = Z5->power(bases, std::vector<long long>{0, 1, 2});
        EXPECT_EQ(raw_int(elemwise[0]), 0);
        EXPECT_EQ(raw_int(elemwise[1]), 2);
        EXPECT_EQ(raw_int(elemwise[2]), 1);
    }

    // Parent checking is the main safety property of runtime Elements:
    // even though both values are integer-backed, an element of Z/7Z cannot
    // be operated on by Z/5Z.
    if (expect_throws([&] {
        (void)Z5->operate(Z5->element(1), Z7->element(1));
    }, "wrong parent in additive group")) return 1;

    // ---------------------------------------------------------------------
    // Runtime multiplicative group (Z/7Z)^*
    //
    // This checks the same runtime Element API for a group whose operation is
    // multiplication modulo n and whose valid elements are units modulo n.
    // ---------------------------------------------------------------------
    auto U7 = std::make_shared<MultiplicativeModGroup>(7);
    EXPECT_EQ(raw_int(U7->identity()), 1);
    EXPECT_TRUE(U7->contains(U7->element(3)));

    // 3^-1 = 5 mod 7 because 3 * 5 = 15 = 1 mod 7.
    EXPECT_EQ(raw_int(U7->inverse(U7->element(3))), 5);
    EXPECT_EQ(raw_int(U7->operate(U7->element(3), U7->element(5))), 1);

    // Multiplicative power uses repeated multiplication:
    //   3^4 = 81 = 4 mod 7
    //   3^-1 = inverse(3) = 5 mod 7
    EXPECT_EQ(raw_int(U7->power(U7->element(3), 4)), 4);
    EXPECT_EQ(raw_int(U7->power(U7->element(3), -1)), 5);

    {
        // Batch multiplicative power with one common exponent:
        //   2^3 = 8 = 1 mod 7
        //   3^3 = 27 = 6 mod 7
        //   4^3 = 64 = 1 mod 7
        std::vector<Element> bases{U7->element(2), U7->element(3), U7->element(4)};
        auto out = U7->power(bases, 3);
        EXPECT_EQ(raw_int(out[0]), 1);
        EXPECT_EQ(raw_int(out[1]), 6);
        EXPECT_EQ(raw_int(out[2]), 1);

        // Batch multiplicative power with one exponent per base.
        auto elemwise = U7->power(bases, std::vector<long long>{0, 1, 2});
        EXPECT_EQ(raw_int(elemwise[0]), 1);
        EXPECT_EQ(raw_int(elemwise[1]), 3);
        EXPECT_EQ(raw_int(elemwise[2]), 2);
    }

    // 2 is not a unit modulo 8 because gcd(2, 8) != 1, so it cannot be a
    // member of the multiplicative group (Z/8Z)^*.
    auto U8 = std::make_shared<MultiplicativeModGroup>(8);
    if (expect_throws([&] {
        (void)U8->element(2);
    }, "non-unit in multiplicative group")) return 1;

    // ---------------------------------------------------------------------
    // Direct sum Z/5Z + Z/7Z
    //
    // DirectSumGroup is the first test of the binding-fix architecture:
    // element data is no longer a fixed C++ pair type. It is a runtime tuple
    // of Elements, so direct sums of any finite arity can be represented by
    // the same Element class.
    // ---------------------------------------------------------------------
    auto G = std::make_shared<DirectSumGroup>(
        std::vector<std::shared_ptr<const Group>>{Z5, Z7}
    );
    EXPECT_TRUE(G->has_property(GroupProperty::Finite));
    EXPECT_TRUE(G->has_property(GroupProperty::Abelian));

    // A direct sum of three finite abelian groups should also be marked
    // finite and abelian. This is the metadata path future utilities will use
    // before running algorithms that require these properties.
    auto ThreeFactorSum = std::make_shared<DirectSumGroup>(
        std::vector<std::shared_ptr<const Group>>{Z5, Z7, U7}
    );
    EXPECT_TRUE(ThreeFactorSum->has_property(GroupProperty::Finite));
    EXPECT_TRUE(ThreeFactorSum->has_property(GroupProperty::Abelian));

    auto g1 = G->element(Element::Vector{Z5->element(2), Z7->element(3)});
    auto g2 = G->element(Element::Vector{Z5->element(4), Z7->element(6)});

    // Componentwise operation:
    //   (2, 3) + (4, 6) = (2+4 mod 5, 3+6 mod 7) = (1, 2)
    auto gsum = G->operate(g1, g2);
    EXPECT_EQ(raw_int(raw_vector(gsum)[0]), 1);
    EXPECT_EQ(raw_int(raw_vector(gsum)[1]), 2);

    // Componentwise inverse:
    //   -(2, 3) = (-2 mod 5, -3 mod 7) = (3, 4)
    auto ginv = G->inverse(g1);
    EXPECT_EQ(raw_int(raw_vector(ginv)[0]), 3);
    EXPECT_EQ(raw_int(raw_vector(ginv)[1]), 4);

    // Componentwise additive power:
    //   3 * (2, 3) = (6 mod 5, 9 mod 7) = (1, 2)
    auto gpow = G->power(g1, 3);
    EXPECT_EQ(raw_int(raw_vector(gpow)[0]), 1);
    EXPECT_EQ(raw_int(raw_vector(gpow)[1]), 2);

    {
        // Batch power for direct sums exercises the generic runtime
        // GroupUtils path over vector-backed Elements.
        std::vector<Element> bases{
            G->element(Element::Vector{Z5->element(1), Z7->element(2)}),
            G->element(Element::Vector{Z5->element(2), Z7->element(3)}),
        };

        auto out = G->power(bases, 3);
        EXPECT_EQ(raw_int(raw_vector(out[0])[0]), 3);
        EXPECT_EQ(raw_int(raw_vector(out[0])[1]), 6);
        EXPECT_EQ(raw_int(raw_vector(out[1])[0]), 1);
        EXPECT_EQ(raw_int(raw_vector(out[1])[1]), 2);

        auto elemwise = G->power(bases, std::vector<long long>{0, 2});
        EXPECT_EQ(raw_int(raw_vector(elemwise[0])[0]), 0);
        EXPECT_EQ(raw_int(raw_vector(elemwise[0])[1]), 0);
        EXPECT_EQ(raw_int(raw_vector(elemwise[1])[0]), 4);
        EXPECT_EQ(raw_int(raw_vector(elemwise[1])[1]), 6);
    }

    // Direct sum element construction must receive exactly one component per
    // factor group.
    if (expect_throws([&] {
        (void)G->element(Element::Vector{Z5->element(1)});
    }, "wrong tuple length in direct sum")) return 1;

    // Component order matters: the first component belongs to Z/5Z and the
    // second belongs to Z/7Z.
    if (expect_throws([&] {
        (void)G->element(Element::Vector{Z7->element(1), Z5->element(1)});
    }, "wrong component parent in direct sum")) return 1;

    // ---------------------------------------------------------------------
    // Nested direct sum: H = (Z/5Z + Z/7Z) + Z/5Z + Z/7Z
    //
    // This is the test that proves we avoided the old binding explosion.
    // The nested element shape is ((a, b), c, d), but C++ still sees one
    // stable runtime Element type instead of Group<pair<pair<int,int>,int>>.
    // ---------------------------------------------------------------------
    auto H = std::make_shared<DirectSumGroup>(
        std::vector<std::shared_ptr<const Group>>{G, Z5, Z7}
    );
    EXPECT_TRUE(H->has_property(GroupProperty::Finite));
    EXPECT_TRUE(H->has_property(GroupProperty::Abelian));

    auto h1 = H->element(Element::Vector{g1, Z5->element(3), Z7->element(4)});
    auto h2 = H->element(Element::Vector{g2, Z5->element(4), Z7->element(5)});
    auto hsum = H->operate(h1, h2);

    // h1 + h2 =
    //   ((2,3), 3,4) + ((4,6), 4,5)
    // = ((1,2), 2,2)
    const auto& hparts = raw_vector(hsum);
    EXPECT_EQ(raw_int(raw_vector(hparts[0])[0]), 1);
    EXPECT_EQ(raw_int(raw_vector(hparts[0])[1]), 2);
    EXPECT_EQ(raw_int(hparts[1]), 2);
    EXPECT_EQ(raw_int(hparts[2]), 2);

    // Print-preview section for manually judging C++/Python-facing repr().
    // These lines are intentionally not assertions; they are visual samples
    // of how elements currently display.
    std::cout << "\nElement repr preview:" << std::endl;
    std::cout << "  Z5 element(8): " << Z5->element(8).repr() << std::endl;
    std::cout << "  U7 element(3): " << U7->element(3).repr() << std::endl;
    std::cout << "  G element (2,3): " << g1.repr() << std::endl;
    std::cout << "  H element ((2,3),3,4): " << h1.repr() << std::endl;
    std::cout << "  H sum: " << hsum.repr() << std::endl;

    std::cout << "Runtime group tests passed." << std::endl;

    // ---------------------------------------------------------------------
    // Runtime ring regression tests
    //
    // Rings now inherit from runtime Group. Their additive group law is wired
    // through zero/add/neg, while multiplication is the extra ring structure.
    // ---------------------------------------------------------------------
    auto R7 = std::make_shared<IntegersModRing>(7);
    EXPECT_EQ(raw_int(R7->zero()), 0);
    EXPECT_EQ(raw_int(R7->one()), 1);
    EXPECT_EQ(raw_int(R7->element(10)), 3);
    EXPECT_EQ(raw_int(R7->add(R7->element(5), R7->element(6))), 4);
    EXPECT_EQ(raw_int(R7->neg(R7->element(3))), 4);
    EXPECT_EQ(raw_int(R7->mul(R7->element(3), R7->element(5))), 1);
    EXPECT_TRUE(R7->element(10).equals(R7->element(3)));
    EXPECT_TRUE(R7->has_property(GroupProperty::Finite));
    EXPECT_TRUE(R7->has_property(GroupProperty::Abelian));

    // Ring::power uses the additive group law of the ring:
    //   3 + 3 + 3 + 3 = 12 = 5 mod 7
    EXPECT_EQ(raw_int(R7->power(R7->element(3), 4)), 5);

    // Ring::mpower uses multiplication:
    //   3^4 = 81 = 4 mod 7
    EXPECT_EQ(raw_int(R7->mpower(R7->element(3), 4)), 4);

    {
        std::vector<Element> bases{R7->element(1), R7->element(2), R7->element(3), R7->element(6)};
        auto out = R7->mpower(bases, 3);
        EXPECT_EQ(raw_int(out[0]), 1);
        EXPECT_EQ(raw_int(out[1]), 1);
        EXPECT_EQ(raw_int(out[2]), 6);
        EXPECT_EQ(raw_int(out[3]), 6);
    }

    // Integer utilities: gcd plus deterministic primality.
    EXPECT_EQ(Integers::gcd(84, 30), 6);
    EXPECT_TRUE(Integers::is_prime(97));
    EXPECT_FALSE(Integers::is_prime(100));

    // Batch primality should preserve order and classify each input.
    std::vector<Integer> nums{2, 4, 5, 9, 97};
    std::vector<bool> expected_primes{true, false, true, false, true};
    EXPECT_VEC_EQ(Integers::is_prime_array(nums), expected_primes);

    std::cout << "Runtime ring tests passed." << std::endl;

    // ---------------------------------------------------------------------
    // Polynomial ring R[X] over a runtime coefficient ring
    //
    // PolynomialRing is a runtime Ring whose elements store coefficients in
    // Element::Vector, interpreted by the parent as increasing-degree
    // coefficients. Elements of the coefficient ring are locally coerced to
    // constant polynomials inside PolynomialRing operations.
    // ---------------------------------------------------------------------
    auto R5 = std::make_shared<IntegersModRing>(5);
    auto PX = std::make_shared<PolynomialRing>(R5, 'X');

    EXPECT_TRUE(PX->has_property(GroupProperty::Abelian));
    EXPECT_FALSE(PX->has_property(GroupProperty::Finite));
    EXPECT_EQ(raw_vector(PX->zero()).size(), static_cast<std::size_t>(0));
    EXPECT_EQ(raw_vector(PX->one()).size(), static_cast<std::size_t>(1));
    EXPECT_EQ(raw_int(raw_vector(PX->one())[0]), 1);

    auto f = PX->element(Element::Vector{R5->element(1), R5->element(2)}); // 1 + 2X
    auto g = PX->element(Element::Vector{R5->element(4), R5->element(1)}); // 4 + X

    // Element creation trims trailing zero coefficients, so
    //   1 + 2X + 0X^2 + 0X^3
    // is stored the same way as 1 + 2X.
    auto with_trailing_zeros = PX->element(
        Element::Vector{R5->element(1), R5->element(2), R5->zero(), R5->zero()}
    );
    EXPECT_TRUE(PX->equals(f, with_trailing_zeros));
    EXPECT_EQ(raw_vector(with_trailing_zeros).size(), static_cast<std::size_t>(2));

    // (1 + 2X) + (4 + X) = 0 + 3X over Z/5Z.
    auto sum = PX->add(f, g);
    EXPECT_EQ(raw_vector(sum).size(), static_cast<std::size_t>(2));
    EXPECT_EQ(raw_int(raw_vector(sum)[0]), 0);
    EXPECT_EQ(raw_int(raw_vector(sum)[1]), 3);

    // -(1 + 2X) = 4 + 3X over Z/5Z.
    auto neg = PX->neg(f);
    EXPECT_EQ(raw_int(raw_vector(neg)[0]), 4);
    EXPECT_EQ(raw_int(raw_vector(neg)[1]), 3);

    // (1 + 2X)(4 + X) = 4 + 4X + 2X^2 over Z/5Z.
    auto product = PX->mul(f, g);
    EXPECT_EQ(raw_vector(product).size(), static_cast<std::size_t>(3));
    EXPECT_EQ(raw_int(raw_vector(product)[0]), 4);
    EXPECT_EQ(raw_int(raw_vector(product)[1]), 4);
    EXPECT_EQ(raw_int(raw_vector(product)[2]), 2);

    // Coercion rule: an element of R is accepted by R[X] operations as a
    // constant polynomial. Element::equals remains strict by parent.
    auto scalar = R5->element(3);
    auto constant_scalar = PX->element(Element::Vector{scalar});
    EXPECT_TRUE(PX->contains(scalar));
    EXPECT_TRUE(PX->equals(scalar, constant_scalar));
    EXPECT_FALSE(scalar.equals(constant_scalar));

    auto add_scalar = PX->add(f, scalar); // (1 + 2X) + 3 = 4 + 2X
    EXPECT_EQ(raw_int(raw_vector(add_scalar)[0]), 4);
    EXPECT_EQ(raw_int(raw_vector(add_scalar)[1]), 2);

    auto mul_scalar = PX->mul(scalar, f); // 3(1 + 2X) = 3 + X
    EXPECT_EQ(raw_int(raw_vector(mul_scalar)[0]), 3);
    EXPECT_EQ(raw_int(raw_vector(mul_scalar)[1]), 1);

    EXPECT_TRUE(PX->equals(R5->zero(), PX->zero()));
    EXPECT_EQ(raw_vector(PX->add(PX->zero(), R5->zero())).size(), static_cast<std::size_t>(0));

    // Generic repr defaults to parenthesizing coefficients.
    auto display = PX->element(Element::Vector{R5->element(2), R5->element(3), R5->element(1)});
    EXPECT_EQ(display.repr(), std::string("(2 mod 5) + (3 mod 5) * X + (1 mod 5) * X^2"));
    EXPECT_EQ(PX->zero().repr(), std::string("0"));
    EXPECT_EQ(constant_scalar.repr(), std::string("(3 mod 5)"));

    // The third constructor argument disables coefficient parentheses.
    auto Pt = std::make_shared<PolynomialRing>(R5, 't', false);
    auto display_no_parens = Pt->element(Element::Vector{R5->element(2), R5->element(3), R5->element(1)});
    EXPECT_EQ(display_no_parens.repr(), std::string("2 mod 5 + 3 mod 5 * t + 1 mod 5 * t^2"));

    auto R7b = std::make_shared<IntegersModRing>(7);
    EXPECT_FALSE(PX->contains(R7b->element(1)));
    if (expect_throws([&] {
        (void)PX->element(Element::Vector{R7b->element(1)});
    }, "wrong coefficient parent in polynomial ring")) return 1;

    if (expect_throws([&] {
        auto bad_variable = std::make_shared<PolynomialRing>(R5, '\0');
        (void)bad_variable;
    }, "null polynomial variable")) return 1;

    std::cout << "Runtime polynomial ring tests passed." << std::endl;
    return 0;
}
