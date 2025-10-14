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

//=====================
//Additive Group mod n: ℤ/nℤ  tests
//=====================

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


//=====================
// MultiplicativeModGroup (ℤ/nℤ)^* tests
//=====================
{
    // Units modulo 7: {1,2,3,4,5,6}
    MultiplicativeModGroup M7(7);

    // Identity
    EXPECT_EQ(M7.identity(), 1);

    // Membership
    for (int a = 1; a < 7; ++a) {
        EXPECT_EQ(M7.contains(a), true);
    }

    // Inverse (3 * 5 ≡ 1 mod 7)
    EXPECT_EQ(M7.inverse(3), 5);
    EXPECT_EQ(M7.operate(3, M7.inverse(3)), M7.identity());

    // Operate (multiplication mod 7)
    EXPECT_EQ(M7.operate(3, 5), 1);  // 15 ≡ 1
    EXPECT_EQ(M7.operate(2, 4), 1);  // 8 ≡ 1

    // GroupUtils power uses the group law (multiplicative here)
    EXPECT_EQ(GroupUtils<int>::power(M7, 3, 0), M7.identity()); // 3^0 = 1
    EXPECT_EQ(GroupUtils<int>::power(M7, 3, 1), 3);
    EXPECT_EQ(GroupUtils<int>::power(M7, 3, 2), 2); // 9 ≡ 2
    EXPECT_EQ(GroupUtils<int>::power(M7, 3, 4), 4); // 81 ≡ 4

    // Negative exponent uses multiplicative inverse
    EXPECT_EQ(GroupUtils<int>::power(M7, 3, -1), M7.inverse(3));  // 3^{-1} ≡ 5
    EXPECT_EQ(GroupUtils<int>::power(M7, 3, -2),
              M7.operate(M7.inverse(3), M7.inverse(3)));          // 5*5 ≡ 4

    // Batch power: same exponent for all
    {
        std::vector<int> bases = {2, 3, 4};
        std::vector<int> expect = {
            1, // 2^3 = 8 ≡ 1
            6, // 3^3 = 27 ≡ 6
            1  // 4^3 = 64 ≡ 1
        };
        EXPECT_VEC_EQ(GroupUtils<int>::power(M7, bases, 3), expect);
    }

    // Batch power: element-wise exponents
    {
        std::vector<int> bases = {2, 3, 5};
        std::vector<int> exps  = {0, 1, 2};
        std::vector<int> expect = {
            1, // 2^0
            3, // 3^1
            4  // 5^2 = 25 ≡ 4
        };
        EXPECT_VEC_EQ(GroupUtils<int>::power(M7, bases, exps), expect);
    }

    // Edge cases: empty input
    {
        std::vector<int> empty;
        EXPECT_VEC_EQ(GroupUtils<int>::power(M7, empty, 5), std::vector<int>{});
    }

    std::cout << "✅ MultiplicativeModGroup(Z/7Z)^* tests passed.\n";
}

{
    // Mod 8: units are {1,3,5,7}; 2 is NOT a unit
    MultiplicativeModGroup M8(8);

    // contains()
    EXPECT_EQ(M8.contains(1), true);
    EXPECT_EQ(M8.contains(3), true);
    EXPECT_EQ(M8.contains(5), true);
    EXPECT_EQ(M8.contains(7), true);
    EXPECT_EQ(M8.contains(2), false);
    EXPECT_EQ(M8.contains(4), false);
    EXPECT_EQ(M8.contains(6), false);

    // inverse() should throw for non-unit (e.g., 2)
    try {
        (void)M8.inverse(2);
        std::cerr << "❌ Expected exception not thrown for inverse(2) in (Z/8Z)^*.\n";
        return 1;
    } catch (const std::invalid_argument&) {
        std::cout << "✅ Caught expected exception for inverse(2) (non-unit).\n";
    }

    // operate() should throw if any operand is not a unit
    try {
        (void)M8.operate(2, 3);
        std::cerr << "❌ Expected exception not thrown for operate(2,3) in (Z/8Z)^*.\n";
        return 1;
    } catch (const std::invalid_argument&) {
        std::cout << "✅ Caught expected exception for operate(2,3) (non-unit involved).\n";
    }

    // Valid operation with units
    EXPECT_EQ(M8.operate(3, 3), 1); // 9 ≡ 1 mod 8

    // Negative exponent via GroupUtils (on units)
    EXPECT_EQ(GroupUtils<int>::power(M8, 3, -1), M8.inverse(3));

    std::cout << "✅ MultiplicativeModGroup(Z/8Z)^* membership/throws tests passed.\n";
}

{
    // Trivial case: mod 1 (everything maps to 0, group has a single element 0)
    MultiplicativeModGroup M1(1);
    EXPECT_EQ(M1.identity(), 0);
    EXPECT_EQ(M1.contains(0), true);
    EXPECT_EQ(M1.contains(42), true); // gcd(42,1)=1
    EXPECT_EQ(M1.inverse(0), 0);      // only element is its own inverse
    EXPECT_EQ(M1.operate(0, 0), 0);
    EXPECT_EQ(GroupUtils<int>::power(M1, 0, 123), 0);
    EXPECT_EQ(GroupUtils<int>::power(M1, 7, -5), 0);

    std::cout << "✅ MultiplicativeModGroup(Z/1Z)^* trivial tests passed.\n";
}







//=====================
// Direct product tests: 
//=====================
{
    auto Gadd5 = std::make_shared<AdditiveModGroup>(5);          // order 5
    auto Gmul7 = std::make_shared<MultiplicativeModGroup>(7);    // order 6
    auto PG = GroupUtils<int>::direct_product<int,int>(Gadd5, Gmul7);

    using P = std::pair<int,int>;

    // identity should be (0, 1)
    P e = PG->identity();
    EXPECT_EQ(e.first,  Gadd5->identity());
    EXPECT_EQ(e.second, Gmul7->identity());

    // inverse: (3,3) -> (2,5)  since -3 ≡ 2 (mod 5), 3^{-1} ≡ 5 (mod 7)
    P x{3,3};
    P ix = PG->inverse(x);
    EXPECT_EQ(ix.first,  Gadd5->inverse(3));
    EXPECT_EQ(ix.second, Gmul7->inverse(3));
    EXPECT_EQ(ix.first,  2);
    EXPECT_EQ(ix.second, 5);

    // operate: (1,3) * (4,5) -> ( (1+4) mod 5, (3*5) mod 7 ) = (0,1)
    P a{1,3}, b{4,5};
    P ab = PG->operate(a,b);
    EXPECT_EQ(ab.first,  0);
    EXPECT_EQ(ab.second, 1);

    // Lagrange's theorem on the product group: |G| = 5*6 = 30
    const int order_PG = 5 * 6;
    for (int aa = 0; aa < 5; ++aa) {
        for (int bb = 1; bb <= 6; ++bb) {
            P g{aa, bb};
            P g_pow = GroupUtils<P>::power(*PG, g, order_PG);
            EXPECT_EQ(g_pow.first,  e.first);
            EXPECT_EQ(g_pow.second, e.second);
        }
    }

    std::cout << "✅ Direct product (Z/5Z,+) × (Z/7Z)^* basic + Lagrange tests passed.\n";
}


{
    auto Gadd4 = std::make_shared<AdditiveModGroup>(4);  // order 4
    auto Gadd6 = std::make_shared<AdditiveModGroup>(6);  // order 6
    auto PG2 = GroupUtils<int>::direct_product<int,int>(Gadd4, Gadd6);

    using P = std::pair<int,int>;
    P e2 = PG2->identity();  // should be (0,0)
    EXPECT_EQ(e2.first, 0);
    EXPECT_EQ(e2.second,0);

    // Sample operations
    P u{3, 5}, v{2, 4};
    P uv = PG2->operate(u, v);   // (3+2 mod 4, 5+4 mod 6) = (1,3)
    EXPECT_EQ(uv.first,  1);
    EXPECT_EQ(uv.second, 3);

    // Inverse in additive groups: negate mod n
    P iu = PG2->inverse(u);  // (-3 mod 4, -5 mod 6) = (1,1)
    EXPECT_EQ(iu.first,  1);
    EXPECT_EQ(iu.second, 1);
    // u + iu = e
    P check = PG2->operate(u, iu);
    EXPECT_EQ(check.first,  e2.first);
    EXPECT_EQ(check.second, e2.second);

    // Lagrange: |G| = 24
    const int order_PG2 = 4 * 6;
    for (int a = 0; a < 4; ++a) {
        for (int b = 0; b < 6; ++b) {
            P g{a, b};
            P g_pow = GroupUtils<P>::power(*PG2, g, order_PG2);
            EXPECT_EQ(g_pow.first,  e2.first);
            EXPECT_EQ(g_pow.second, e2.second);
        }
    }

    std::cout << "✅ Direct product (Z/4Z,+) × (Z/6Z,+) basic + Lagrange tests passed.\n";
}


{
    auto Gadd5 = std::make_shared<AdditiveModGroup>(5);
    auto Gmul7 = std::make_shared<MultiplicativeModGroup>(7);
    auto PG = GroupUtils<int>::direct_product<int,int>(Gadd5, Gmul7);

    using P = std::pair<int,int>;
    P g{2, 3};
    int k = 11; // arbitrary exponent

    // Power in the product
    P gk = GroupUtils<P>::power(*PG, g, k);

    // Power in each component, then pair
    int left  = GroupUtils<int>::power(*Gadd5, 2, k); // repeated addition mod 5
    int right = GroupUtils<int>::power(*Gmul7, 3, k); // multiplication mod 7

    EXPECT_EQ(gk.first,  left);
    EXPECT_EQ(gk.second, right);

    std::cout << "✅ Product power equals component-wise power test passed.\n";
}










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


{
    // 8) Deterministic primality: scalars and batch
    Integers Z;

    // ---- Scalar checks ----
    EXPECT_EQ(Z.is_prime(-5), false);
    EXPECT_EQ(Z.is_prime(0),  false);
    EXPECT_EQ(Z.is_prime(1),  false);
    EXPECT_EQ(Z.is_prime(4),  false);
    EXPECT_EQ(Z.is_prime(9),  false);
    EXPECT_EQ(Z.is_prime(21), false);
    EXPECT_EQ(Z.is_prime(221), false);               // 13 * 17
    EXPECT_EQ(Z.is_prime(1000000000000LL), false);   // 10^12, composite

    EXPECT_EQ(Z.is_prime(2),  true);
    EXPECT_EQ(Z.is_prime(3),  true);
    EXPECT_EQ(Z.is_prime(5),  true);
    EXPECT_EQ(Z.is_prime(7),  true);
    EXPECT_EQ(Z.is_prime(11), true);
    EXPECT_EQ(Z.is_prime(97), true);
    EXPECT_EQ(Z.is_prime(1000000007LL), true);       // 1e9+7 (prime)

    // ---- Batch check ----
    std::vector<long long> nums = {
        2, 4, 5, 9, 1, 97, 1000000007LL, 1000000000000LL
    };
    std::vector<bool> expected = {
        true, false, true, false, false, true, true, false
    };
    std::vector<bool> flags = Integers::is_prime_array(nums);

    EXPECT_VEC_EQ(flags, expected);

    std::cout << "✅ Primality (scalar + batch) passed.\n";
}


//  9) Polynomials over Z: Z[x]

{
    PolynomialsOverIntegers P;

    // 9.1) Zero / One / degree / is_zero
    {
        Poly z = P.zero();           // {}
        Poly o = P.one();            // {1}
        EXPECT_EQ(PolynomialsOverIntegers::degree(z), -1);
        EXPECT_EQ(PolynomialsOverIntegers::degree(o), 0);
        EXPECT_EQ(PolynomialsOverIntegers::is_zero(z), true);
        EXPECT_EQ(PolynomialsOverIntegers::is_zero(o), false);

        // zero should be canonical empty vector
        EXPECT_VEC_EQ(z, Poly{});
        EXPECT_VEC_EQ(o, Poly{1});

        std::cout << "✅ Poly 9.1 (zero/one/degree/is_zero) passed.\n";
    }

    // 9.2) Add / Neg / Sub (basic)
    {
        Poly f{3, 2, 1};      // 3 + 2x + x^2
        Poly g{1, -5};        // 1 - 5x
        Poly sum = P.add(f, g);     // (3+1) + (2-5)x + 1x^2 = 4 - 3x + x^2
        Poly negf = P.neg(f);       // -3 -2x - x^2
        Poly f_minus_g = P.sub(f, g); // (3-1) + (2-(-5))x + x^2 = 2 + 7x + x^2

        EXPECT_VEC_EQ(sum, Poly({4, -3, 1}));
        EXPECT_VEC_EQ(negf, Poly({-3, -2, -1}));
        EXPECT_VEC_EQ(f_minus_g, Poly({2, 7, 1}));

        std::cout << "✅ Poly 9.2 (add/neg/sub) passed.\n";
    }

    // 9.3) Naive multiplication (small polys)
    {
        // (1 + 2x) * (3 + 4x) = 3 + 10x + 8x^2
        Poly a{1, 2};
        Poly b{3, 4};
        Poly prod = P.mul_naive(a, b);
        EXPECT_VEC_EQ(prod, Poly({3, 10, 8}));

        // (5 - x + 2x^2) * (1 + x) = 5 + 4x + x^2 + 2x^3
        Poly c{5, -1, 2};
        Poly d{1, 1};
        Poly prod2 = P.mul_naive(c, d);
        EXPECT_VEC_EQ(prod2, Poly({5, 4, 1, 2}));

        std::cout << "✅ Poly 9.3 (naive mul small) passed.\n";
    }

    // 9.4) Multiplication and trimming of trailing zeros
    {
        // (0 + 0x + 5x^2) * (0 + 1x) = 0 + 0x + 0x^2 + 5x^3
        Poly a{0, 0, 5};
        Poly b{0, 1};
        Poly prod = P.mul_naive(a, b);
        EXPECT_VEC_EQ(prod, Poly({0, 0, 0, 5})); // canonical keeps leading 5 at degree 3

        // Trimming on add: [1,2,0,0] == [1,2]
        Poly e{1, 2, 0, 0};
        Poly f{1, 2};
        EXPECT_EQ(P.is_equal(e, f), true);

        std::cout << "✅ Poly 9.4 (mul + trimming) passed.\n";
    }

    // 9.5) Distributivity: f*(g+h) = f*g + f*h (check with naive)
    {
        Poly f{2, 0, 1};   // 2 + x^2
        Poly g{1, 3};      // 1 + 3x
        Poly h{-1, 4, 1};  // -1 + 4x + x^2

        Poly gh = P.add(g, h);
        Poly lhs = P.mul_naive(f, gh);
        Poly rhs = P.add(P.mul_naive(f, g), P.mul_naive(f, h));
        EXPECT_VEC_EQ(lhs, rhs);

        std::cout << "✅ Poly 9.5 (distributivity with naive) passed.\n";
    }

    // 9.6) Default mul == naive mul (sanity)
    {
        Poly a{4, -2, 0, 7};   // 4 - 2x + 7x^3
        Poly b{3, 1, 5};       // 3 + x + 5x^2
        Poly m_def = P.mul(a, b);
        Poly m_nv  = P.mul_naive(a, b);
        EXPECT_VEC_EQ(m_def, m_nv);

        std::cout << "✅ Poly 9.6 (mul == mul_naive) passed.\n";
    }

    // 9.7) Karatsuba vs Naive equivalence (medium polys)
    {
        // Deterministic “random-looking” coefficients
        Poly a(50), b(45);
        for (std::size_t i = 0; i < a.size(); ++i) a[i] = static_cast<std::int64_t>((int)(i % 7) - 3); // in [-3..3]
        for (std::size_t j = 0; j < b.size(); ++j) b[j] = static_cast<std::int64_t>((int)(2*(j % 5)) - 4); // in [-4..4]

        // ensure some non-trivial ends
        a[0] = 5; a[1] = -2; a.back() = 3;
        b[0] = -1; b[1] = 4; b.back() = -2;

        Poly mk = P.mul_karatsuba(a, b);
        Poly mn = P.mul_naive(a, b);
        EXPECT_VEC_EQ(mk, mn);

        std::cout << "✅ Poly 9.7 (karatsuba == naive) passed.\n";
    }

    // 9.8) Identity behavior: f*1 = f, f*0 = 0 (both mul variants)
    {
        Poly f{3, -1, 0, 2};  // 3 - x + 2x^3
        Poly one = P.one();
        Poly zero = P.zero();

        EXPECT_VEC_EQ(P.mul_naive(f, one), f);
        EXPECT_VEC_EQ(P.mul_karatsuba(f, one), f);

        EXPECT_VEC_EQ(P.mul_naive(f, zero), Poly{});
        EXPECT_VEC_EQ(P.mul_karatsuba(f, zero), Poly{});

        std::cout << "✅ Poly 9.8 (identity/annihilator) passed.\n";
    }

    // 9.9) Pretty-print (to_string) and canonical equality
    {
        EXPECT_EQ(PolynomialsOverIntegers::to_string(Poly{}), "0");
        EXPECT_EQ(PolynomialsOverIntegers::to_string(Poly{1}), "1");
        EXPECT_EQ(PolynomialsOverIntegers::to_string(Poly{0, -1, 2}), "-x + 2x^2");
        EXPECT_EQ(PolynomialsOverIntegers::to_string(Poly{3, -1, 0, 1}), "3 - x + x^3");

        // canonical equality ignores trailing zeros
        Poly a{1, 2, 0, 0};
        Poly b{1, 2};
        EXPECT_EQ(P.is_equal(a, b), true);

        // sub agrees with add+neg
        Poly f{2, 5, -3};
        Poly g{1, -1, 4};
        EXPECT_VEC_EQ(P.sub(f, g), P.add(f, P.neg(g)));

        std::cout << "✅ Poly 9.9 (to_string + canonical equality + sub) passed.\n";
    }
}




    return 0;
}