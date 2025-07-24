#ifndef PRIMEPY_ALGEBRAIC_STRUCTURES_H
#define PRIMEPY_ALGEBRAIC_STRUCTURES_H

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
    // Additive group: identity, inverse, operate already exist
    virtual T zero() const = 0;
    virtual T one() const = 0;

    virtual T add(const T& a, const T& b) const = 0;
    virtual T neg(const T& a) const = 0;

    virtual T mul(const T& a, const T& b) const = 0;

    virtual bool is_equal(const T& a, const T& b) const = 0;
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

#endif // PRIMEPY_ALGEBRAIC_STRUCTURES_H