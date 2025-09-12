#include <vector>
#include <iostream>
#include <string>
#include <exception>

#include "../include/primepy/algebra/algebraic_structures_exp.h"
#include "../include/primepy/algebra/algebraic_structures_utils.h"

using namespace primepy::algebra;

// Simple comparison macro with debug info
#define EXPECT_EQ(val, expected) \
    if ((val) != (expected)) { \
        std::cerr << "❌ Test failed at " << __FILE__ << ":" << __LINE__ \
                  << "\nExpected: " << (expected) << ", Got: " << (val) << std::endl; \
        return 1; \
    }

// Helper to check vector equality
template<typename T>
bool vectors_equal(const std::vector<T>& a, const std::vector<T>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) return false;
    return true;
}

#define EXPECT_VEC_EQ(v1, v2) \
    if (!vectors_equal((v1), (v2))) { \
        std::cerr << "❌ Vector test failed at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::cerr << "Expected: ["; \
        for (auto x : (v2)) std::cerr << x << " "; \
        std::cerr << "], Got: ["; \
        for (auto x : (v1)) std::cerr << x << " "; \
        std::cerr << "]" << std::endl; \
        return 1; \
    }

int main() {


//=======================================================================================
// Groups
//=======================================================================================



    AdditiveModGroup G(7);

    // Scalar tests
    EXPECT_EQ(G.identity(), 0);
    EXPECT_EQ(G.inverse(3), 4);
    EXPECT_EQ(G.operate(3, 6), 2);
    EXPECT_EQ(GroupUtils<int>::power(G, 2, 3), 6);
    EXPECT_EQ(GroupUtils<int>::power(G, 2, 0), 0);
    EXPECT_EQ(GroupUtils<int>::power(G, 2, 1), 2);
    EXPECT_EQ(GroupUtils<int>::power(G, 2, -1), G.inverse(2));
    EXPECT_EQ(GroupUtils<int>::power(G, 3, -2), G.operate(G.inverse(3), G.inverse(3)));

    // Vector test (same exponent)
    std::vector<int> inputs = {1, 2, 3};
    std::vector<int> expected_pow3 = {
        G.operate(G.operate(1, 1), 1),
        G.operate(G.operate(2, 2), 2),
        G.operate(G.operate(3, 3), 3)
    };
    EXPECT_VEC_EQ(GroupUtils<int>::power(G, inputs, 3), expected_pow3);

    // Vector test (element-wise exponents)
    std::vector<int> exponents = {0, 1, 2};
    std::vector<int> expected_elemwise = {
        G.identity(),
        2,
        G.operate(3, 3)
    };
    EXPECT_VEC_EQ(GroupUtils<int>::power(G, inputs, exponents), expected_elemwise);

    // Identity + inverse = identity
    for (int a = 0; a < 7; ++a) {
        EXPECT_EQ(G.operate(a, G.inverse(a)), G.identity());
    }

    // Edge cases
    {
        std::vector<int> empty;
        EXPECT_VEC_EQ(GroupUtils<int>::power(G, empty, 3), std::vector<int>{});
    }

    {
        std::vector<int> one_elem = {5};
        std::vector<int> one_exp = {-2};
        std::vector<int> result = GroupUtils<int>::power(G, one_elem, one_exp);
        EXPECT_VEC_EQ(result, std::vector<int>{G.operate(G.inverse(5), G.inverse(5))});
    }

    {
        try {
            std::vector<int> bad_exp = {1, 2}; // size mismatch
            auto _ = GroupUtils<int>::power(G, inputs, bad_exp);
            std::cerr << "❌ Expected exception not thrown for mismatched vectors.\n";
            return 1;
        } catch (const std::invalid_argument& e) {
            std::cout << "✅ Caught expected exception for mismatched input sizes.\n";
        }
    }

    std::cout << "✅ All AdditiveModGroup & power tests passed.\n";




//=======================================================================================
// Rings
//=======================================================================================

{
    // 1) Basics on ℤ/7ℤ (ring primitives + scalar power)
    IntegersModRing R(7);

    // ring primitives
    EXPECT_EQ(R.zero(), 0);
    EXPECT_EQ(R.one(), 1);
    EXPECT_EQ(R.add(5, 6), 4);   // 11 ≡ 4 (mod 7)
    EXPECT_EQ(R.neg(3), 4);      // -3 ≡ 4
    EXPECT_EQ(R.mul(3, 5), 1);   // 15 ≡ 1

    // equality & membership
    EXPECT_EQ(R.is_equal(10, 3), true);  // 10 ≡ 3
    EXPECT_EQ(R.contains(12345), true);

    // multiplicative power (scalar)
    EXPECT_EQ(RingUtils<int>::power(R, 3, 0), R.one());     // a^0 = 1
    EXPECT_EQ(RingUtils<int>::power(R, 3, 1), 3);
    EXPECT_EQ(RingUtils<int>::power(R, 3, 2), R.mul(3,3));  // 9 ≡ 2
    EXPECT_EQ(RingUtils<int>::power(R, 3, 2), 2);
    EXPECT_EQ(RingUtils<int>::power(R, 3, 4), 4);           // 81 ≡ 4

    std::cout << "✅ IntegersModRing(7) basics & scalar power passed.\n";
}

{
    // 2) Batch power (same exponent for all)
    IntegersModRing R(7);
    std::vector<int> bases = {1, 2, 3, 6};
    auto out = RingUtils<int>::power(R, bases, 3);  // cube each

    // expected: a^3 mod 7
    std::vector<int> exp = {
        1,                             // 1^3
        R.mul(R.mul(2,2),2),           // 8 ≡ 1
        R.mul(R.mul(3,3),3),           // 27 ≡ 6
        R.mul(R.mul(6,6),6)            // 216 ≡ 6
    };
    EXPECT_VEC_EQ(out, exp);
    std::cout << "✅ RingUtils batch(same exponent) passed.\n";
}

{
    // 3) Batch power (element-wise exponents)
    IntegersModRing R(7);
    std::vector<int> bases = {2, 3, 4, 5};
    std::vector<long long> exps = {0, 1, 2, 3};  // use long long to match RingUtils

    auto out = RingUtils<int>::power(R, bases, exps);

    std::vector<int> exp = {
        R.one(),                   // 2^0 = 1
        3,                         // 3^1
        R.mul(4,4),                // 16 ≡ 2
        R.mul(R.mul(5,5),5)        // 125 ≡ 6
    };
    EXPECT_VEC_EQ(out, exp);
    std::cout << "✅ RingUtils batch(element-wise) passed.\n";
}

{
    // 4) Negative exponent should throw (general rings)
    IntegersModRing R(7);
    try {
        (void)RingUtils<int>::power(R, 3, -1);
        std::cerr << "❌ Expected exception not thrown for negative exponent.\n";
        return 1;
    } catch (const std::invalid_argument&) {
        std::cout << "✅ Caught expected exception for negative exponent (rings).\n";
    }
}

{
    // 5) Contrast additive vs multiplicative power on a ring
    IntegersModRing R(7);
    // GroupUtils uses the ADDITIVE structure (since Ring<T> : Group<T>)
    // 3 (+) 3 (+) 3 (+) 3  ≡ 12 ≡ 5
    EXPECT_EQ(GroupUtils<int>::power(R, 3, 4), 5);

    // RingUtils uses MULTIPLICATION
    // 3^4 ≡ 81 ≡ 4
    EXPECT_EQ(RingUtils<int>::power(R, 3, 4), 4);

    std::cout << "✅ Additive vs multiplicative power on ring passed.\n";
}

{
    // 6) Zero ring ℤ/1ℤ edge cases
    IntegersModRing R1(1);
    EXPECT_EQ(R1.zero(), 0);
    EXPECT_EQ(R1.one(), R1.zero());                 // 0 = 1 in ℤ/1ℤ
    EXPECT_EQ(R1.add(123, 456), 0);
    EXPECT_EQ(R1.mul(123, 456), 0);
    EXPECT_EQ(RingUtils<int>::power(R1, 5, 123456789LL), 0); // anything^n = 0 here
    EXPECT_EQ(R1.is_equal(7, 0), true);
    std::cout << "✅ Zero ring (mod 1) tests passed.\n";
}

{
    // 7) Distributivity spot checks
    IntegersModRing R(7);
    int a = 3, b = 5, c = 6;
    int lhs1 = R.mul(a, R.add(b, c));
    int rhs1 = R.add(R.mul(a, b), R.mul(a, c));
    EXPECT_EQ(lhs1, rhs1);

    int lhs2 = R.mul(R.add(a, b), c);
    int rhs2 = R.add(R.mul(a, c), R.mul(b, c));
    EXPECT_EQ(lhs2, rhs2);

    std::cout << "✅ Distributivity spot checks passed.\n";
}

    return 0;
}