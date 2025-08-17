#ifndef PRIMEPY_GROUP_UTILS_H
#define PRIMEPY_GROUP_UTILS_H

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

        #pragma omp parallel for ifdef _OPENMP
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

        #pragma omp parallel for ifdef _OPENMP
        for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
            results[i] = power(group, bases[i], exponents[i]);
        }
        return results;
    }
};

} // namespace primepy::algebra

#endif // PRIMEPY_GROUP_UTILS_H