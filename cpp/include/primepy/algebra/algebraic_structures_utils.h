#pragma once

#include "algebraic_structures.h"
#include <vector>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace primepy::algebra {

//=============================
// Group utils
//=============================
template<typename T>
class GroupUtils {
public:
    // Scalar version
    static T power(const Group<T>& group, T base, int exponent) {
        if (exponent < 0) {
            base = group.inverse(base);
            exponent = -exponent;
        }

        T result = group.identity();
        while (exponent > 0) {
            if (exponent & 1) {
                result = group.operate(result, base);
            }
            base = group.operate(base, base);
            exponent >>= 1;
        }
        return result;
    }

    // Batch version: same exponent for all
    static std::vector<T> power(const Group<T>& group, const std::vector<T>& bases, int exponent) {
        std::vector<T> results(bases.size());

        #ifdef _OPENMP
        #pragma omp parallel for
        #endif
        for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
            results[i] = power(group, bases[i], exponent);
        }
        return results;
    }

    // Batch version: element-wise exponentiation
    static std::vector<T> power(const Group<T>& group, const std::vector<T>& bases, const std::vector<int>& exponents) {
        if (bases.size() != exponents.size()) {
            throw std::invalid_argument("Size of bases and exponents must match.");
        }

        std::vector<T> results(bases.size());

        #ifdef _OPENMP
        #pragma omp parallel for
        #endif
        for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
            results[i] = power(group, bases[i], exponents[i]);
        }
        return results;
    }
};









//=============================
// Ring utils (multiplicative power)
//=============================
template<typename T>
class RingUtils {
public:
    // Scalar: a^n with n >= 0 (binary exponentiation)
    static T power(const Ring<T>& ring, T base, long long exponent) {
        if (exponent < 0) {
            throw std::invalid_argument(
                        "RingUtils::power: negative exponent is not supported for general rings. "
        "If this structure is a division ring/field, support for negative exponents "
        "will be added in a future version.");
        }
        T result = ring.one();
        while (exponent > 0) {
            if (exponent & 1LL) {
                result = ring.mul(result, base);
            }
            exponent >>= 1LL;
            if (exponent) base = ring.mul(base, base);
        }
        return result;
    }

    // Batch: same exponent for all elements (n >= 0)
    static std::vector<T> power(const Ring<T>& ring,
                                const std::vector<T>& bases,
                                long long exponent)
    {
        if (exponent < 0) {
            throw std::invalid_argument(
                        "RingUtils::power: negative exponent is not supported for general rings. "
        "If this structure is a division ring/field, support for negative exponents "
        "will be added in a future version.");
        }

        std::vector<T> results(bases.size());

        #ifdef _OPENMP
        #pragma omp parallel for
        #endif
        for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
            results[i] = power(ring, bases[i], exponent);
        }
        return results;
    }

    // Batch: element-wise exponents (each n_i >= 0)
    static std::vector<T> power(const Ring<T>& ring,
                                const std::vector<T>& bases,
                                const std::vector<long long>& exponents)
    {
        if (bases.size() != exponents.size()) {
            throw std::invalid_argument("RingUtils::power(vec, vec): sizes must match.");
        }

        std::vector<T> results(bases.size());

        #ifdef _OPENMP
        #pragma omp parallel for
        #endif
        for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
            if (exponents[i] < 0) {
                // we disallow negatives.
                throw std::invalid_argument("RingUtils::power: negative exponent is not supported for general rings. "
        "If this structure is a division ring/field, support for negative exponents "
        "will be added in a future version." + std::to_string(i));
            }
            results[i] = power(ring, bases[i], exponents[i]);
        }
        return results;
    }
};







} // namespace primepy::algebra

