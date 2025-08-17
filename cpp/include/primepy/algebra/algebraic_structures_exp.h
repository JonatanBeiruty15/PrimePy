#ifndef PRIMEPY_ALGEBRAIC_STRUCTURES_EXP_H
#define PRIMEPY_ALGEBRAIC_STRUCTURES_EXP_H

#include "algebraic_structures.h"
#include <vector>
#include <algorithm>

namespace primepy::algebra {

//=============================
// Additive Group mod n: ℤ/nℤ
//=============================
class AdditiveModGroup : public Group<int> {
public:
    explicit AdditiveModGroup(int modulus);

    int identity() const override;
    int inverse(const int& a) const override;
    int operate(const int& a, const int& b) const override;

private:
    int mod;
};

//=============================
// Multiplicative Group mod p (ℤ/pℤ)^*
//=============================


//TODO

//=============================
// Symmetric Group Sₙ finite
//=============================
//TODO

} // namespace primepy::algebra

#endif // PRIMEPY_ALGEBRAIC_STRUCTURES_EXP_H