#pragma once
#include "algebraic_structures.h"
#include <vector>
#include <algorithm>



namespace primepy::algebra {

//=======================================================================================
// Groups
//=======================================================================================
    

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
// Multiplicative Group mod n (ℤ/nℤ)^*
//=============================

// class MultiplicativeModGroup : public Group<int> {
// public:
//     explicit MultiplicativeModGroup (int modulus);

//     int identity() const override;
//     int inverse(const int& a) const override;
//     int operate(const int& a, const int& b) const override;
//     bool contains(const int& a) const ; // gcd(a, mod) == 1

// private:
//     int mod;
    
// };
//TODO

//=============================
// Symmetric Group Sₙ finite
//=============================
//TODO







//=======================================================================================
// Rings
//=======================================================================================


//=============================
// Ring of integers modulo n: ℤ/nℤ
//=============================
class IntegersModRing : public Ring<int> {
public:
    explicit IntegersModRing(int modulus);

    // --------- Ring primitives ---------
    int zero() const override;                          // 0
    int one()  const override;                          // 1
    int add(const int& a, const int& b) const override; // (a + b) mod n
    int neg(const int& a) const override;               // (-a) mod n
    int mul(const int& a, const int& b) const override; // (a * b) mod n


    bool is_equal(const int& a, const int& b) const override; // a ≡ b (mod n)
    bool contains(const int& a) const ;               // always true for ℤ/nℤ

    // --------- Utility ---------
    int modulus() const { return mod; }

private:
    int mod;  // the modulus n
    inline int normalize(int x) const {
        int r = x % mod;
        return r < 0 ? r + mod : r;   // ensures result in [0, mod-1]
    }
};







} // namespace primepy::algebra

