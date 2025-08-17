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
    return 0;
}