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

MultiplicativeModGroup::MultiplicativeModGroup(int modulus) : mod(modulus) {
    if (modulus <= 0) {
        throw std::invalid_argument("Modulus must be positive.");
    }
}

int MultiplicativeModGroup::identity() const {
    // In Z/1Z the only class is 0, so 1 % 1 == 0 (still correct)
    return 1 % mod;
}

int MultiplicativeModGroup::inverse(const int& a) const {
    // Normalize a into [0, mod-1]
    auto norm = [this](int x) {
        int r = x % mod;
        return (r < 0) ? r + mod : r;
    };
    int a0 = norm(a);

    // Membership check via contains()
    if (!contains(a0)) {
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
    return t;
}

int MultiplicativeModGroup::operate(const int& a, const int& b) const {
    // Enforce membership using contains()
    if (!contains(a) || !contains(b)) {
        throw std::invalid_argument(
            "MultiplicativeModGroup::operate: both operands must be units modulo the modulus.");
    }

    // Normalize first to keep behavior clean with negatives
    auto norm = [this](int x) {
        int r = x % mod;
        return (r < 0) ? r + mod : r;
    };

    long long a0 = norm(a);
    long long b0 = norm(b);
    return static_cast<int>((a0 * b0) % mod);
}

bool MultiplicativeModGroup::contains(const int& a) const {
    return Integers::gcd(a, mod) == 1;
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


int Integers::zero() const {
    return 0;
}

int Integers::one() const {
    return 1;
}

int Integers::add(const int& a, const int& b) const {
    // Plain integer addition (note: will overflow in int if very large)
    return a + b;
}

int Integers::neg(const int& a) const {
    return -a;
}

int Integers::mul(const int& a, const int& b) const {
    // Plain integer multiplication (note: will overflow in int if very large)
    return a * b;
}

// ----- Hooks -----
bool Integers::is_equal(const int& a, const int& b) const {
    return a == b;
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
bool Integers::is_prime(std::int64_t n) {
    if (n < 2) return false;
    return is_prime_u64(static_cast<std::uint64_t>(n));
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
std::vector<bool> Integers::is_prime_array(const std::vector<std::int64_t>& nums) {
    std::vector<bool> results(nums.size());

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
        results[i] = is_prime(nums[i]);
    }

    return results;
}





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






//=============================
// Ring of Polynomials over Z; Z[X] in one variable
//=============================

// ----------- internal helpers for fast multiplication -----------
namespace detail {

    // Local add (no vtable call). Works on raw Poly and trims.
    inline Poly poly_add(const Poly& a, const Poly& b) {
        Poly r(std::max(a.size(), b.size()), 0);
        for (std::size_t i = 0; i < r.size(); ++i) {
            __int128 v = 0;
            if (i < a.size()) v += a[i];
            if (i < b.size()) v += b[i];
            r[i] = static_cast<std::int64_t>(v);
        }
        trim_trailing_zeros(r);
        return r;
    }

    // Local sub (no vtable call). r = a - b; trims.
    inline Poly poly_sub(const Poly& a, const Poly& b) {
        Poly r(std::max(a.size(), b.size()), 0);
        for (std::size_t i = 0; i < r.size(); ++i) {
            __int128 v = 0;
            if (i < a.size()) v += a[i];
            if (i < b.size()) v -= b[i];
            r[i] = static_cast<std::int64_t>(v);
        }
        trim_trailing_zeros(r);
        return r;
    }

    // Naive O(n*m) multiply with widened accumulation.
    inline Poly naive_mul_raw(const Poly& f, const Poly& g) {
        if (f.empty() || g.empty()) return {};
        Poly result(f.size() + g.size() - 1, 0);
        for (std::size_t i = 0; i < f.size(); ++i) {
            for (std::size_t j = 0; j < g.size(); ++j) {
                __int128 acc = static_cast<__int128>(result[i + j])
                             + static_cast<__int128>(f[i]) * static_cast<__int128>(g[j]);
                result[i + j] = static_cast<std::int64_t>(acc);
            }
        }
        trim_trailing_zeros(result);
        return result;
    }

    // Karatsuba multiply (recursive). Base-cases fall back to naive.
    inline Poly karatsuba_mul_raw(const Poly& a, const Poly& b) {
        if (a.empty() || b.empty()) return {};

        const std::size_t n = a.size();
        const std::size_t m = b.size();

        // Base threshold; tune later if desired.
        if (n <= 32 || m <= 32) {
            return naive_mul_raw(a, b);
        }

        const std::size_t k = std::min(n, m) / 2;

        // Split a = a0 + x^k a1 ; b = b0 + x^k b1
        Poly a0(a.begin(), a.begin() + std::min(k, n));
        Poly a1(a.begin() + std::min(k, n), a.end());
        Poly b0(b.begin(), b.begin() + std::min(k, m));
        Poly b1(b.begin() + std::min(k, m), b.end());
        trim_trailing_zeros(a0); trim_trailing_zeros(a1);
        trim_trailing_zeros(b0); trim_trailing_zeros(b1);

        // z0 = a0*b0
        Poly z0 = karatsuba_mul_raw(a0, b0);
        // z2 = a1*b1
        Poly z2 = karatsuba_mul_raw(a1, b1);
        // z1 = (a0+a1)*(b0+b1) - z0 - z2
        Poly a0a1 = poly_add(a0, a1);
        Poly b0b1 = poly_add(b0, b1);
        Poly z1   = karatsuba_mul_raw(a0a1, b0b1);
        z1 = poly_sub(z1, z0);
        z1 = poly_sub(z1, z2);

        // Combine: res = z0 + (z1 << k) + (z2 << 2k)
        Poly res = z0;

        auto ensure = [&](std::size_t sz) {
            if (res.size() < sz) res.resize(sz, 0);
        };

        // add z1 shifted by k
        ensure(std::max(res.size(), z1.size() + k));
        for (std::size_t i = 0; i < z1.size(); ++i) {
            __int128 acc = static_cast<__int128>(res[i + k]) + z1[i];
            res[i + k] = static_cast<std::int64_t>(acc);
        }

        // add z2 shifted by 2k
        ensure(std::max(res.size(), z2.size() + 2 * k));
        for (std::size_t i = 0; i < z2.size(); ++i) {
            __int128 acc = static_cast<__int128>(res[i + 2 * k]) + z2[i];
            res[i + 2 * k] = static_cast<std::int64_t>(acc);
        }

        trim_trailing_zeros(res);
        return res;
    }

} // namespace detail

// -------------------- ring primitives ------------------------

Poly PolynomialsOverIntegers::zero() const { return Poly{}; }      // 0
Poly PolynomialsOverIntegers::one()  const { return Poly{1}; }     // 1

Poly PolynomialsOverIntegers::add(const Poly& f, const Poly& g) const {
    const std::size_t n = std::max(f.size(), g.size());
    Poly result(n, 0);
    for (std::size_t i = 0; i < n; ++i) {
        const std::int64_t a = (i < f.size() ? f[i] : 0);
        const std::int64_t b = (i < g.size() ? g[i] : 0);
        result[i] = a + b;
    }
    trim_trailing_zeros(result);
    return result;
}

Poly PolynomialsOverIntegers::neg(const Poly& f) const {
    Poly result(f.size());
    for (std::size_t i = 0; i < f.size(); ++i) result[i] = -f[i];
    trim_trailing_zeros(result);
    return result;
}

// Default public multiply = naive (keeps Ring<T>::mul implemented)
Poly PolynomialsOverIntegers::mul(const Poly& f, const Poly& g) const {
    return detail::naive_mul_raw(f, g);
}

// Explicit variants for external control (e.g., Python)
Poly PolynomialsOverIntegers::mul_naive(const Poly& f, const Poly& g) const {
    return detail::naive_mul_raw(f, g);
}

Poly PolynomialsOverIntegers::mul_karatsuba(const Poly& f, const Poly& g) const {
    return detail::karatsuba_mul_raw(f, g);
}

// ---------------- equality + membership hooks ----------------

bool PolynomialsOverIntegers::is_equal(const Poly& f, const Poly& g) const {
    Poly a = f, b = g;
    trim_trailing_zeros(a);
    trim_trailing_zeros(b);
    return a == b;
}

bool PolynomialsOverIntegers::contains(const Poly& /*f*/) const {
    // Any std::vector<int64_t> is a valid element in this representation.
    return true;
}

// ------------------------ pretty print -----------------------

std::string PolynomialsOverIntegers::to_string(const Poly& f) {
    Poly p = f;
    trim_trailing_zeros(p);
    if (p.empty()) return "0";

    std::ostringstream out;
    bool first = true;

    for (std::size_t i = 0; i < p.size(); ++i) {
        const auto c = p[i];
        if (c == 0) continue;

        // sign
        if (!first) out << (c > 0 ? " + " : " - ");
        else if (c < 0) out << "-";

        // abs coeff (skip printing 1 for non-constant terms)
        const auto abs_c = std::llabs(c);
        if (!(abs_c == 1 && i > 0)) out << abs_c;

        // x^i
        if (i >= 1) out << "x";
        if (i >= 2) out << "^" << i;

        first = false;
    }

    return out.str();
}


} // namespace primepy::algebra