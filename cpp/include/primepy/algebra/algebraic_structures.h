#pragma once

#include <stdexcept>
#include <string>
#include <memory>

namespace primepy::algebra {

//=============================
// Abstract Group Interface
//=============================
template<typename T>
class Group {
public:
    virtual T identity() const = 0;
    virtual T inverse(const T& a) const = 0;
    virtual T operate(const T& a, const T& b) const = 0;

    virtual ~Group() = default;
};

//=============================
// Abstract Ring Interface
//=============================
template<typename T>
class Ring : public Group<T> {
public:
    using value_type = T;
    virtual ~Ring() = default;

    // --------- Ring primitives every concrete ring must provide ---------
    virtual T zero() const = 0;                          // additive identity
    virtual T one()  const = 0;                          // multiplicative identity
    virtual T add(const T& a, const T& b) const = 0;     // a + b
    virtual T neg(const T& a) const = 0;                 // -a
    virtual T mul(const T& a, const T& b) const = 0;     // a * b

    // --------- Optional hooks (override when needed) ---------
    // Equality in the ring (e.g., congruence mod n). Default: raw ==
    virtual bool is_equal(const T& a, const T& b) const { return a == b; }


    // --------- Additive Group<T> interface (wired once here) ---------
    T identity() const override { return zero(); }                     // e_add = 0
    T inverse(const T& a) const override { return neg(a); }            // a^{-1}_add = -a
    T operate(const T& a, const T& b) const override { return add(a, b); } // a ⊕ b = a + b
};



//=============================
// Abstract Field Interface
//=============================
template<typename T>
class Field : public Ring<T> {
public:
    virtual T div(const T& a, const T& b) const = 0;
    virtual T inv(const T& a) const = 0;
};

} // namespace primepy::algebra

