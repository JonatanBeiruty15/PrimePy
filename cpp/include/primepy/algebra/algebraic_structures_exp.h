#pragma once
#include "algebraic_structures.h"
#include <vector>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>


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






// =============================
// Multiplicative Group mod n (ℤ/nℤ)^*
// =============================

class MultiplicativeModGroup : public Group<int> {
public:
    explicit MultiplicativeModGroup (int modulus);

    int identity() const override;
    int inverse(const int& a) const override;
    int operate(const int& a, const int& b) const override;
    bool contains(const int& a) const ; // gcd(a, mod) == 1

private:
    int mod;
    
};


//=============================
// Symmetric Group Sₙ finite
//=============================
//TODO







//=======================================================================================
// Rings
//=======================================================================================


//=============================
// Ring of integers: ℤ
//=============================
class Integers : public Ring<int> {
public:
    // No parameters needed: it's just ℤ
    Integers() = default;

    // ----- Ring primitives -----
    int zero() const override;                         // 0
    int one()  const override;                         // 1
    int add(const int& a, const int& b) const override;// a + b
    int neg(const int& a) const override;              // -a
    int mul(const int& a, const int& b) const override;// a * b

    
    bool is_equal(const int& a, const int& b) const override; // a == b
    bool contains(const int& /*a*/) const { return true; }   // any int is in ℤ

    // gcd utility 
    static int gcd(int a, int b);

    // ---- Deterministic primality (bounded) ----
    // Correct for all 64-bit unsigned integers (and thus all signed 63-bit positives).
    static bool is_prime(std::int64_t n);
    static bool is_prime_u64(std::uint64_t n);
    // ---- Batch primality ----
    static std::vector<bool> is_prime_array(const std::vector<std::int64_t>& nums);

private:
    // Helpers for Miller–Rabin
    static std::uint64_t mod_mul(std::uint64_t a, std::uint64_t b, std::uint64_t m);
    static std::uint64_t mod_pow(std::uint64_t a, std::uint64_t e, std::uint64_t m);
    static bool miller_rabin_witness(std::uint64_t a, std::uint64_t n);

};






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


//=============================
// Ring of Polynomials over Z; Z[X] in one variable
//=============================

/// Polynomial over Z represented as coefficients in increasing degree order:
/// p(x) = coeffs[0] + coeffs[1] * x + ... + coeffs[n] * x^n
using Poly = std::vector<std::int64_t>;

/// Utility: remove trailing zeros so the representation is canonical.
inline void trim_trailing_zeros(Poly& p) {
    while (!p.empty() && p.back() == 0) p.pop_back();
}

/// Ring of polynomials Z[x] (one variable).
/// Elements are Poly (std::vector<int64_t>) with canonical trimming.
class PolynomialsOverIntegers final : public Ring<Poly> {
public:
    PolynomialsOverIntegers() = default;

    // ---- Ring primitives over Poly (required overrides) ----
    Poly zero() const override;                          // 0
    Poly one()  const override;                          // 1

    Poly add(const Poly& f, const Poly& g) const override; // f + g
    Poly neg(const Poly& f) const override;                // -f

    /// Default multiplication. Kept as the Ring<T>::mul override.
    /// (Defined to call the naive version by default.)
    Poly mul(const Poly& f, const Poly& g) const override; // f * g

    // ---- Extra public API (explicit control from Python) ----

    /// Schoolbook multiplication (O(n*m)).
    Poly mul_naive(const Poly& f, const Poly& g) const;

    /// Karatsuba multiplication (O(n^{log_2 3})) — faster for large polys.
    Poly mul_karatsuba(const Poly& f, const Poly& g) const;

    /// Subtraction convenience: f - g (implemented as add(f, neg(g))).
    Poly sub(const Poly& f, const Poly& g) const {
        return add(f, neg(g));
    }

    /// Degree of polynomial (−1 for the zero polynomial).
    static int degree(const Poly& f) {
        Poly tmp = f;
        trim_trailing_zeros(tmp);
        return tmp.empty() ? -1 : static_cast<int>(tmp.size()) - 1;
    }

    /// Zero check in canonical form.
    static bool is_zero(const Poly& f) {
        Poly tmp = f;
        trim_trailing_zeros(tmp);
        return tmp.empty();
    }

    /// Ensure canonical form (remove trailing zeros).
    static void normalize(Poly& f) { trim_trailing_zeros(f); }

    /// Pretty print, e.g. "3 + 2x^2 - x^5".
    static std::string to_string(const Poly& f);

    // ---- Equality hook (override if your Ring<T> exposes it) ----
    bool is_equal(const Poly& f, const Poly& g) const override;

    /// Membership check (always true for this representation).
    bool contains(const Poly& f) const;

    ///todo: find roots, decompose into prime factors.
};




} // namespace primepy::algebra

