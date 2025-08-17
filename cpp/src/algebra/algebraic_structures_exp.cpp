#include "primepy/algebra/algebraic_structures_exp.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace primepy::algebra {

//=====================
// AdditiveModGroup
//=====================
AdditiveModGroup::AdditiveModGroup(int modulus) : mod(modulus) {
    if (modulus <= 0) {
        throw std::invalid_argument("Modulus must be positive.");
    }
}

int AdditiveModGroup::identity() const {
    return 0;
}

int AdditiveModGroup::inverse(const int& a) const {
    return (mod - (a % mod)) % mod;
}

int AdditiveModGroup::operate(const int& a, const int& b) const {
    return (a + b) % mod;
}

//=====================
// MultiplicativeModGroup
//=====================


//TODO

//=====================
// SymmetricGroupfinite
//=====================

//TODO


} // namespace primepy::algebra