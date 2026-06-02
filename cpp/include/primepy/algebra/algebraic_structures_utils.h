#pragma once

// Project headers
#include "algebraic_structures.h"   // Group, Ring, etc.

// Standard headers
#include <memory>    // std::shared_ptr
#include <utility>   // std::move, std::pair
#include <vector>    // std::vector
#include <stdexcept> // std::invalid_argument

// parallelization
#ifdef _OPENMP
#include <omp.h>
#endif

namespace primepy::algebra {

//=============================
// Runtime Group utils
//=============================
class GroupUtils {
public:
    static Element power(const Group& group, Element base, long long exponent);

    static std::vector<Element> power(const Group& group,
                                      const std::vector<Element>& bases,
                                      long long exponent);

    static std::vector<Element> power(const Group& group,
                                      const std::vector<Element>& bases,
                                      const std::vector<long long>& exponents);

    static std::shared_ptr<DirectSumGroup>
    direct_sum(std::vector<std::shared_ptr<const Group>> factors);
};

} // namespace primepy::algebra
