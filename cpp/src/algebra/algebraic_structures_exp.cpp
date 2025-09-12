#include "primepy/algebra/algebraic_structures_exp.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace primepy::algebra {


//=======================================================================================
// Groups
//=======================================================================================

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

// MultiplicativeModGroup::MultiplicativeModGroup(int modulus) : mod(modulus) {
//     if (modulus <= 0) {
//         throw std::invalid_argument("Modulus must be positive.");
//     }
// }

// int MultiplicativeModGroup::identity() const {
//     return 1;
// }

// bool MultiplicativeModGroup::contains(const int& a) const {
//     throw std::logic_error("contains() not yet implemented");
// }


// int MultiplicativeModGroup::inverse(const int& a) const {
//     throw std::logic_error("contains() not yet implemented");
// }





//=====================
// SymmetricGroupfinite
//=====================

//TODO








//=======================================================================================
// Rings
//=======================================================================================

//=============================
// Ring of integers modulo n: ℤ/nℤ
//=============================


IntegersModRing::IntegersModRing(int modulus)
    : mod(modulus)
{
    if (modulus <= 0) {
        throw std::invalid_argument("IntegersModRing: modulus must be positive.");
    }
}

// --------- Ring primitives ---------
int IntegersModRing::zero() const {
    // Additive identity class
    return 0;
}

int IntegersModRing::one() const {
    // Multiplicative identity class [1]; for n=1 this is also 0
    return normalize(1);
}

int IntegersModRing::add(const int& a, const int& b) const {
    long long x = normalize(a);
    long long y = normalize(b);
    return normalize(static_cast<int>(x + y));
}

int IntegersModRing::neg(const int& a) const {
    // (-a) mod n
    long long v = - static_cast<long long>(a);
    return normalize(static_cast<int>(v % mod));
}

int IntegersModRing::mul(const int& a, const int& b) const {
    int x = normalize(a);
    int y = normalize(b);
    long long p = static_cast<long long>(x) * static_cast<long long>(y);
    return normalize(static_cast<int>(p % mod));
}


bool IntegersModRing::is_equal(const int& a, const int& b) const {
    // a ≡ b (mod n)  <=>  normalize(a - b) == 0
    long long d = static_cast<long long>(a) - static_cast<long long>(b);
    int r = static_cast<int>(d % mod);
    return normalize(r) == 0;
}

bool IntegersModRing::contains(const int& /*a*/) const {
    // Every integer represents a residue class modulo n
    return true;
}








} // namespace primepy::algebra