#include "primepy/algebra/algebraic_structures_exp.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <array>


namespace primepy::algebra {

namespace {

Integer integer_from_long_long(long long value) {
    return Integer(std::to_string(value));
}

int normalize_mod_int(const Integer& value, int modulus) {
    Integer r = value % modulus;
    if (r < 0) {
        r += modulus;
    }
    return r.get_si();
}

} // namespace

//=======================================================================================
// Groups
//=======================================================================================

//=====================
// AdditiveModGroup
//=====================
AdditiveModGroup::AdditiveModGroup(int modulus)
    : Group({GroupProperty::Finite, GroupProperty::Abelian}), mod(modulus)
{
    if (modulus <= 0) {
        throw std::invalid_argument("Modulus must be positive.");
    }
}

int AdditiveModGroup::normalize(const Integer& x) const {
    return normalize_mod_int(x, mod);
}

Element AdditiveModGroup::element(const Integer& value) const {
    return Element(self(), normalize(value));
}

Element AdditiveModGroup::identity() const {
    return Element(self(), 0);
}

Element AdditiveModGroup::inverse(const Element& a) const {
    require_parent(a, "AdditiveModGroup::inverse");
    return Element(self(), normalize(-std::get<Integer>(a.data())));
}

Element AdditiveModGroup::operate(const Element& a, const Element& b) const {
    require_same_parent(a, b, "AdditiveModGroup::operate");
    return Element(self(), normalize(
        std::get<Integer>(a.data()) + std::get<Integer>(b.data())
    ));
}

bool AdditiveModGroup::contains(const Element& a) const {
    return a.parent().get() == this && std::holds_alternative<Integer>(a.data());
}

bool AdditiveModGroup::equals(const Element& a, const Element& b) const {
    require_same_parent(a, b, "AdditiveModGroup::equals");
    return normalize(std::get<Integer>(a.data())) == normalize(std::get<Integer>(b.data()));
}

std::string AdditiveModGroup::repr(const Element& a) const {
    require_parent(a, "AdditiveModGroup::repr");
    return std::to_string(normalize(std::get<Integer>(a.data()))) + " mod " + std::to_string(mod);
}

Element AdditiveModGroup::power(const Element& base, long long exponent) const {
    require_parent(base, "AdditiveModGroup::power");
    return Element(self(), normalize(std::get<Integer>(base.data()) * integer_from_long_long(exponent)));
}

//=====================
// MultiplicativeModGroup
//=====================

MultiplicativeModGroup::MultiplicativeModGroup(int modulus)
    : Group({GroupProperty::Finite, GroupProperty::Abelian}), mod(modulus)
{
    if (modulus <= 0) {
        throw std::invalid_argument("Modulus must be positive.");
    }
}

int MultiplicativeModGroup::normalize(const Integer& x) const {
    return normalize_mod_int(x, mod);
}

bool MultiplicativeModGroup::contains_integer(const Integer& a) const {
    return Integers::gcd(normalize(a), mod) == 1;
}

Element MultiplicativeModGroup::element(const Integer& value) const {
    int v = normalize(value);
    if (!contains_integer(v)) {
        throw std::invalid_argument(
            "MultiplicativeModGroup::element: element is not a unit modulo the modulus.");
    }
    return Element(self(), v);
}

Element MultiplicativeModGroup::identity() const {
    // In Z/1Z the only class is 0, so 1 % 1 == 0 (still correct)
    return Element(self(), 1 % mod);
}

Element MultiplicativeModGroup::inverse(const Element& a) const {
    require_parent(a, "MultiplicativeModGroup::inverse");
    int a0 = normalize(std::get<Integer>(a.data()));

    // Membership check via contains()
    if (!contains_integer(a0)) {
        throw std::invalid_argument(
            "MultiplicativeModGroup::inverse: element not in the group (not coprime to modulus).");
    }

    // Extended Euclidean Algorithm to find a0^{-1} mod mod
    int t = 0, new_t = 1;
    int r = mod, new_r = a0;

    while (new_r != 0) {
        int q = r / new_r;

        int temp_t = t - q * new_t;
        t = new_t;
        new_t = temp_t;

        int temp_r = r - q * new_r;
        r = new_r;
        new_r = temp_r;
    }

    // r == 1 here because contains(a0) guaranteed gcd(a0, mod) == 1
    if (t < 0) t += mod;
    return Element(self(), normalize(t));
}

Element MultiplicativeModGroup::operate(const Element& a, const Element& b) const {
    require_same_parent(a, b, "MultiplicativeModGroup::operate");

    // Enforce membership using contains()
    if (!contains(a) || !contains(b)) {
        throw std::invalid_argument(
            "MultiplicativeModGroup::operate: both operands must be units modulo the modulus.");
    }

    int a0 = normalize(std::get<Integer>(a.data()));
    int b0 = normalize(std::get<Integer>(b.data()));
    long long product = static_cast<long long>(a0) * static_cast<long long>(b0);
    return Element(self(), normalize(integer_from_long_long(product)));
}

bool MultiplicativeModGroup::contains(const Element& a) const {
    return a.parent().get() == this
        && std::holds_alternative<Integer>(a.data())
        && contains_integer(std::get<Integer>(a.data()));
}

bool MultiplicativeModGroup::equals(const Element& a, const Element& b) const {
    require_same_parent(a, b, "MultiplicativeModGroup::equals");
    return normalize(std::get<Integer>(a.data())) == normalize(std::get<Integer>(b.data()));
}

std::string MultiplicativeModGroup::repr(const Element& a) const {
    require_parent(a, "MultiplicativeModGroup::repr");
    return std::to_string(normalize(std::get<Integer>(a.data()))) + " mod " + std::to_string(mod);
}




//=====================
// SymmetricGroupfinite
//=====================

//TODO








//=======================================================================================
// Rings
//=======================================================================================


//=============================
// Ring of integers: ℤ
//=============================

Integers::Integers()
    : Ring({GroupProperty::Abelian})
{
}

Element Integers::element(const Integer& value) const {
    return Element(self(), value);
}

Element Integers::zero() const {
    return Element(self(), 0);
}

Element Integers::one() const {
    return Element(self(), 1);
}

Element Integers::add(const Element& a, const Element& b) const {
    require_same_parent(a, b, "Integers::add");
    return Element(self(), std::get<Integer>(a.data()) + std::get<Integer>(b.data()));
}

Element Integers::neg(const Element& a) const {
    require_parent(a, "Integers::neg");
    return Element(self(), -std::get<Integer>(a.data()));
}

Element Integers::mul(const Element& a, const Element& b) const {
    require_same_parent(a, b, "Integers::mul");
    return Element(self(), std::get<Integer>(a.data()) * std::get<Integer>(b.data()));
}

bool Integers::contains(const Element& a) const {
    return a.parent().get() == this && std::holds_alternative<Integer>(a.data());
}

bool Integers::equals(const Element& a, const Element& b) const {
    require_same_parent(a, b, "Integers::equals");
    return std::get<Integer>(a.data()) == std::get<Integer>(b.data());
}

std::string Integers::repr(const Element& a) const {
    require_parent(a, "Integers::repr");
    return std::get<Integer>(a.data()).get_str();
}

// ----- gcd (Euclidean algorithm; returns nonnegative gcd; gcd(0,0)=0) -----
int Integers::gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    if (a == 0) return b;
    if (b == 0) return a;
    while (b != 0) {
        int r = a % b;
        a = b;
        b = r;
    }
    return a; // nonnegative
}


// ----- Modular arithmetic helpers -----
std::uint64_t Integers::mod_mul(std::uint64_t a, std::uint64_t b, std::uint64_t m) {
#if defined(__SIZEOF_INT128__)
    return static_cast<std::uint64_t>((__uint128_t)a * b % m);
#else
    // Portable fallback: add-and-double (O(log b)), overflow-safe.
    std::uint64_t res = 0;
    a %= m;
    while (b) {
        if (b & 1ULL) {
            res += a;
            if (res >= m) res -= m;
        }
        b >>= 1ULL;
        a <<= 1ULL;
        if (a >= m) a -= m;
    }
    return res;
#endif
}

std::uint64_t Integers::mod_pow(std::uint64_t a, std::uint64_t e, std::uint64_t m) {
    std::uint64_t r = 1 % m;
    a %= m;
    while (e) {
        if (e & 1ULL) r = mod_mul(r, a, m);
        a = mod_mul(a, a, m);
        e >>= 1ULL;
    }
    return r;
}

// One Miller–Rabin round for base a
bool Integers::miller_rabin_witness(std::uint64_t a, std::uint64_t n) {
    if (a % n == 0) return true; // base divisible by n -> trivial pass
    // write n-1 = d * 2^s with d odd
    std::uint64_t d = n - 1, s = 0;
    while ((d & 1ULL) == 0ULL) { d >>= 1ULL; ++s; }

    std::uint64_t x = mod_pow(a, d, n);
    if (x == 1 || x == n - 1) return true;
    for (std::uint64_t i = 1; i < s; ++i) {
        x = mod_mul(x, x, n);
        if (x == n - 1) return true;
    }
    return false; // composite for this base
}




// ---- Public API: deterministic primality ----
bool Integers::is_prime(const Integer& n) {
    if (n < 2) return false;
    if (mpz_fits_ulong_p(n.get_mpz_t())) {
        return is_prime_u64(n.get_ui());
    }
    return mpz_probab_prime_p(n.get_mpz_t(), 25) != 0;
}

bool Integers::is_prime_u64(std::uint64_t n) {
    if (n < 2) return false;

    // Small primes & even check
    static constexpr std::array<std::uint64_t, 12> small{
        2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL
    };
    for (auto p : small) {
        if (n == p) return true;
        if (n % p == 0ULL) return (n == p);
    }

    // Deterministic Miller–Rabin:
    // For n < 2^64, testing bases {2,3,5,7,11,13,17,19,23,29,31,37} is proven sufficient.
    // (Smaller set {2,3,5,7,11} also works for all n < 2^32.)
    const std::array<std::uint64_t, 12> bases64{2,3,5,7,11,13,17,19,23,29,31,37};
    // Micro-optimization: if n < 2^32, fewer bases suffice
    if (n < (1ULL << 32)) {
        const std::array<std::uint64_t, 5> bases32{2,3,5,7,11};
        for (auto a : bases32) if (!miller_rabin_witness(a, n)) return false;
        return true;
    }
    for (auto a : bases64) if (!miller_rabin_witness(a, n)) return false;
    return true;
}


// ---- Batch primality ----
std::vector<bool> Integers::is_prime_array(const std::vector<Integer>& nums) {
    std::vector<unsigned char> tmp(nums.size());

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
        tmp[i] = is_prime(nums[i]) ? 1 : 0;
    }

    std::vector<bool> results(nums.size());
    for (std::size_t i = 0; i < tmp.size(); ++i) {
        results[i] = (tmp[i] != 0);
    }

    return results;
}





//=============================
// Ring of integers modulo n: ℤ/nℤ
//=============================


IntegersModRing::IntegersModRing(int modulus)
    : Ring({GroupProperty::Finite, GroupProperty::Abelian}), mod(modulus)
{
    if (modulus <= 0) {
        throw std::invalid_argument("IntegersModRing: modulus must be positive.");
    }
}

// --------- Ring primitives ---------
Element IntegersModRing::element(const Integer& value) const {
    return Element(self(), normalize(value));
}

Element IntegersModRing::zero() const {
    // Additive identity class
    return Element(self(), 0);
}

Element IntegersModRing::one() const {
    // Multiplicative identity class [1]; for n=1 this is also 0
    return Element(self(), normalize(1));
}

Element IntegersModRing::add(const Element& a, const Element& b) const {
    require_same_parent(a, b, "IntegersModRing::add");
    int x = normalize(std::get<Integer>(a.data()));
    int y = normalize(std::get<Integer>(b.data()));
    long long sum = static_cast<long long>(x) + static_cast<long long>(y);
    return Element(self(), normalize(integer_from_long_long(sum)));
}

Element IntegersModRing::neg(const Element& a) const {
    require_parent(a, "IntegersModRing::neg");
    // (-a) mod n
    Integer v = -std::get<Integer>(a.data());
    return Element(self(), normalize(v));
}

Element IntegersModRing::mul(const Element& a, const Element& b) const {
    require_same_parent(a, b, "IntegersModRing::mul");
    int x = normalize(std::get<Integer>(a.data()));
    int y = normalize(std::get<Integer>(b.data()));
    long long p = static_cast<long long>(x) * static_cast<long long>(y);
    return Element(self(), normalize(integer_from_long_long(p)));
}


bool IntegersModRing::contains(const Element& a) const {
    return a.parent().get() == this && std::holds_alternative<Integer>(a.data());
}

bool IntegersModRing::equals(const Element& a, const Element& b) const {
    require_same_parent(a, b, "IntegersModRing::equals");
    // a ≡ b (mod n)  <=>  normalize(a - b) == 0
    Integer d = std::get<Integer>(a.data()) - std::get<Integer>(b.data());
    return normalize(d) == 0;
}

std::string IntegersModRing::repr(const Element& a) const {
    require_parent(a, "IntegersModRing::repr");
    return std::to_string(normalize(std::get<Integer>(a.data()))) + " mod " + std::to_string(mod);
}


//=============================
// Polynomial ring in one variable: R[X]
//=============================

PolynomialRing::PolynomialRing(std::shared_ptr<const Ring> coefficient_ring,
                               char variable,
                               bool use_parentheses)
    : Ring(infer_properties(coefficient_ring)),
      coefficient_ring_(std::move(coefficient_ring)),
      variable_(variable),
      use_parentheses_(use_parentheses)
{
    if (!coefficient_ring_) {
        throw std::invalid_argument("PolynomialRing: coefficient ring must be non-null.");
    }
    if (variable_ == '\0') {
        throw std::invalid_argument("PolynomialRing: variable must be a non-null character.");
    }
}

std::vector<GroupProperty>
PolynomialRing::infer_properties(const std::shared_ptr<const Ring>& coefficient_ring) {
    if (!coefficient_ring) {
        throw std::invalid_argument("PolynomialRing: coefficient ring must be non-null.");
    }

    std::vector<GroupProperty> properties;
    if (coefficient_ring->has_property(GroupProperty::Abelian)) {
        properties.push_back(GroupProperty::Abelian);
    }
    return properties;
}

bool PolynomialRing::is_zero_coefficient(const Element& coefficient) const {
    if (!coefficient_ring_->contains(coefficient)) {
        throw std::invalid_argument("PolynomialRing: coefficient belongs to the wrong ring.");
    }
    return coefficient.equals(coefficient_ring_->zero());
}

void PolynomialRing::trim(Element::Vector& coefficients) const {
    while (!coefficients.empty() && is_zero_coefficient(coefficients.back())) {
        coefficients.pop_back();
    }
}

Element::Vector PolynomialRing::coefficients_for(const Element& f) const {
    if (f.parent().get() == this) {
        if (!std::holds_alternative<Element::Vector>(f.data())) {
            throw std::invalid_argument("PolynomialRing: polynomial element must store vector data.");
        }

        Element::Vector coefficients = std::get<Element::Vector>(f.data());
        for (const auto& coefficient : coefficients) {
            if (!coefficient_ring_->contains(coefficient)) {
                throw std::invalid_argument(
                    "PolynomialRing: polynomial coefficient belongs to the wrong coefficient ring.");
            }
        }
        trim(coefficients);
        return coefficients;
    }

    if (f.parent().get() == coefficient_ring_.get()) {
        if (!coefficient_ring_->contains(f)) {
            throw std::invalid_argument(
                "PolynomialRing: coefficient element is not contained in the coefficient ring.");
        }
        if (is_zero_coefficient(f)) {
            return {};
        }
        return Element::Vector{f};
    }

    throw std::invalid_argument(
        "PolynomialRing: expected an element of this polynomial ring or its coefficient ring.");
}

Element PolynomialRing::from_coefficients(Element::Vector coefficients) const {
    for (const auto& coefficient : coefficients) {
        if (!coefficient_ring_->contains(coefficient)) {
            throw std::invalid_argument(
                "PolynomialRing: coefficient belongs to the wrong coefficient ring.");
        }
    }
    trim(coefficients);
    return Element(self(), std::move(coefficients));
}

std::string PolynomialRing::format_coefficient(const Element& coefficient) const {
    const std::string text = coefficient.repr();
    return use_parentheses_ ? "(" + text + ")" : text;
}

Element PolynomialRing::element(const Element::Vector& coefficients) const {
    return from_coefficients(coefficients);
}

Element PolynomialRing::zero() const {
    return Element(self(), Element::Vector{});
}

Element PolynomialRing::one() const {
    return from_coefficients(Element::Vector{coefficient_ring_->one()});
}

Element PolynomialRing::add(const Element& f, const Element& g) const {
    auto a = coefficients_for(f);
    auto b = coefficients_for(g);

    const std::size_t n = std::max(a.size(), b.size());
    Element::Vector out;
    out.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        Element ai = (i < a.size()) ? a[i] : coefficient_ring_->zero();
        Element bi = (i < b.size()) ? b[i] : coefficient_ring_->zero();
        out.push_back(coefficient_ring_->add(ai, bi));
    }

    return from_coefficients(std::move(out));
}

Element PolynomialRing::neg(const Element& f) const {
    auto a = coefficients_for(f);
    Element::Vector out;
    out.reserve(a.size());

    for (const auto& coefficient : a) {
        out.push_back(coefficient_ring_->neg(coefficient));
    }

    return from_coefficients(std::move(out));
}

Element PolynomialRing::mul(const Element& f, const Element& g) const {
    auto a = coefficients_for(f);
    auto b = coefficients_for(g);

    if (a.empty() || b.empty()) {
        return zero();
    }

    Element::Vector out(a.size() + b.size() - 1, coefficient_ring_->zero());

    for (std::size_t i = 0; i < a.size(); ++i) {
        for (std::size_t j = 0; j < b.size(); ++j) {
            Element product = coefficient_ring_->mul(a[i], b[j]);
            out[i + j] = coefficient_ring_->add(out[i + j], product);
        }
    }

    return from_coefficients(std::move(out));
}

bool PolynomialRing::contains(const Element& f) const {
    if (f.parent().get() == coefficient_ring_.get()) {
        return coefficient_ring_->contains(f);
    }

    if (f.parent().get() != this || !std::holds_alternative<Element::Vector>(f.data())) {
        return false;
    }

    const auto& coefficients = std::get<Element::Vector>(f.data());
    for (const auto& coefficient : coefficients) {
        if (!coefficient_ring_->contains(coefficient)) {
            return false;
        }
    }
    return true;
}

bool PolynomialRing::equals(const Element& f, const Element& g) const {
    auto a = coefficients_for(f);
    auto b = coefficients_for(g);

    if (a.size() != b.size()) {
        return false;
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!a[i].equals(b[i])) {
            return false;
        }
    }
    return true;
}

std::string PolynomialRing::repr(const Element& f) const {
    auto coefficients = coefficients_for(f);
    if (coefficients.empty()) {
        return "0";
    }

    std::ostringstream out;
    bool first = true;

    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        if (is_zero_coefficient(coefficients[i])) {
            continue;
        }

        if (!first) {
            out << " + ";
        }

        out << format_coefficient(coefficients[i]);
        if (i >= 1) {
            out << " * " << variable_;
        }
        if (i >= 2) {
            out << "^" << i;
        }

        first = false;
    }

    return first ? std::string("0") : out.str();
}
} // namespace primepy::algebra
