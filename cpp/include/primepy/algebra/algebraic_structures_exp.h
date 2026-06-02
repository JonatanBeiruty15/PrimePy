#pragma once
#include "algebraic_structures.h"
#include <vector>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <gmpxx.h>


namespace primepy::algebra {

//=======================================================================================
// Groups
//=======================================================================================
    

//=============================
// Additive Group mod n: ℤ/nℤ
//=============================
class AdditiveModGroup : public Group {
public:
    using Group::power;

    explicit AdditiveModGroup(int modulus);

    Element element(const Integer& value) const override;
    Element identity() const override;
    Element inverse(const Element& a) const override;
    Element operate(const Element& a, const Element& b) const override;
    bool contains(const Element& a) const override;
    bool equals(const Element& a, const Element& b) const override;
    std::string repr(const Element& a) const override;
    Element power(const Element& base, long long exponent) const override;

    int modulus() const { return mod; }

private:
    int mod;
    int normalize(const Integer& x) const;
};






// =============================
// Multiplicative Group mod n (ℤ/nℤ)^*
// =============================

class MultiplicativeModGroup : public Group {
public:
    explicit MultiplicativeModGroup (int modulus);

    Element element(const Integer& value) const override;
    Element identity() const override;
    Element inverse(const Element& a) const override;
    Element operate(const Element& a, const Element& b) const override;
    bool contains(const Element& a) const override; // gcd(a, mod) == 1
    bool equals(const Element& a, const Element& b) const override;
    std::string repr(const Element& a) const override;

    int modulus() const { return mod; }

private:
    int mod;
    int normalize(const Integer& x) const;
    bool contains_integer(const Integer& a) const;
    
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
class Integers : public Ring {
public:
    using Ring::power;
    using Ring::mpower;

    // No parameters needed: it's just ℤ
    Integers();

    // ----- Ring primitives -----
    Element element(const Integer& value) const override;
    Element zero() const override;                         // 0
    Element one()  const override;                         // 1
    Element add(const Element& a, const Element& b) const override;// a + b
    Element neg(const Element& a) const override;              // -a
    Element mul(const Element& a, const Element& b) const override;// a * b

    
    bool contains(const Element& a) const override;
    bool equals(const Element& a, const Element& b) const override;
    std::string repr(const Element& a) const override;

    // gcd utility 
    static int gcd(int a, int b);

    // ---- Deterministic primality (bounded) ----
    // Correct for all 64-bit unsigned integers (and thus all signed 63-bit positives).
    static bool is_prime(const Integer& n);
    static bool is_prime_u64(std::uint64_t n);
    // ---- Batch primality ----
    static std::vector<bool> is_prime_array(const std::vector<Integer>& nums);

private:
    // Helpers for Miller–Rabin
    static std::uint64_t mod_mul(std::uint64_t a, std::uint64_t b, std::uint64_t m);
    static std::uint64_t mod_pow(std::uint64_t a, std::uint64_t e, std::uint64_t m);
    static bool miller_rabin_witness(std::uint64_t a, std::uint64_t n);

};






//=============================
// Ring of integers modulo n: ℤ/nℤ
//=============================
class IntegersModRing : public Ring {
public:
    using Ring::power;
    using Ring::mpower;

    explicit IntegersModRing(int modulus);

    // --------- Ring primitives ---------
    Element element(const Integer& value) const override;
    Element zero() const override;                          // 0
    Element one()  const override;                          // 1
    Element add(const Element& a, const Element& b) const override; // (a + b) mod n
    Element neg(const Element& a) const override;               // (-a) mod n
    Element mul(const Element& a, const Element& b) const override; // (a * b) mod n


    bool contains(const Element& a) const override;
    bool equals(const Element& a, const Element& b) const override;
    std::string repr(const Element& a) const override;

    // --------- Utility ---------
    int modulus() const { return mod; }

private:
    int mod;  // the modulus n
    inline int normalize(const Integer& x) const {
        Integer r = x % mod;
        if (r < 0) {
            r += mod;
        }
        return r.get_si();   // result is in [0, mod-1]
    }
};


//=============================
// Polynomial ring in one variable: R[X]
//=============================
class PolynomialRing : public Ring {
public:
    using Ring::power;
    using Ring::mpower;

    explicit PolynomialRing(std::shared_ptr<const Ring> coefficient_ring,
                            char variable = 'X',
                            bool use_parentheses = true);

    Element element(const Element::Vector& coefficients) const override;
    Element zero() const override;
    Element one() const override;
    Element add(const Element& f, const Element& g) const override;
    Element neg(const Element& f) const override;
    Element mul(const Element& f, const Element& g) const override;

    bool contains(const Element& f) const override;
    bool equals(const Element& f, const Element& g) const override;
    std::string repr(const Element& f) const override;

    const std::shared_ptr<const Ring>& coefficient_ring() const { return coefficient_ring_; }
    char variable() const { return variable_; }

private:
    std::shared_ptr<const Ring> coefficient_ring_;
    char variable_;
    bool use_parentheses_;

    static std::vector<GroupProperty> infer_properties(const std::shared_ptr<const Ring>& coefficient_ring);
    bool is_zero_coefficient(const Element& coefficient) const;
    void trim(Element::Vector& coefficients) const;
    Element::Vector coefficients_for(const Element& f) const;
    Element from_coefficients(Element::Vector coefficients) const;
    std::string format_coefficient(const Element& coefficient) const;
};


} // namespace primepy::algebra
