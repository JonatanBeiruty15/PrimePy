#pragma once

#include <stdexcept>
#include <string>
#include <memory>
#include <variant>
#include <vector>
#include <cstddef>
#include <gmpxx.h>

namespace primepy::algebra {

using Integer = mpz_class;

enum class GroupProperty {
    Finite,
    Abelian,
};

//=============================
// Runtime Group Element
//=============================
class Group;

class Element {
public:
    using Vector = std::vector<Element>;
    using Data = std::variant<Integer, Vector>;

    Element(std::shared_ptr<const Group> parent, Integer value);
    Element(std::shared_ptr<const Group> parent, long value);
    Element(std::shared_ptr<const Group> parent, int value);
    Element(std::shared_ptr<const Group> parent, Vector value);

    const std::shared_ptr<const Group>& parent() const { return parent_; }
    const Data& data() const { return data_; }

    bool equals(const Element& other) const;
    std::string repr() const;

private:
    std::shared_ptr<const Group> parent_;
    Data data_;
};




//=============================
// Runtime Group Interface
//=============================
class Group : public std::enable_shared_from_this<Group> {
    friend class GroupUtils;
public:
    bool has_property(GroupProperty property) const;
    const std::vector<GroupProperty>& properties() const { return properties_; }

    virtual Element element(const Integer& value) const;
    virtual Element element(const Element::Vector& values) const;

    virtual Element identity() const = 0;
    virtual Element inverse(const Element& a) const = 0;
    virtual Element operate(const Element& a, const Element& b) const = 0;
    virtual bool contains(const Element& a) const;
    virtual bool equals(const Element& a, const Element& b) const = 0;
    virtual std::string repr(const Element& a) const = 0;
    virtual Element power(const Element& base, long long exponent) const;
    virtual std::vector<Element> power(const std::vector<Element>& bases, long long exponent) const;
    virtual std::vector<Element> power(const std::vector<Element>& bases, const std::vector<long long>& exponents) const;

    virtual ~Group() = default;

protected:
    explicit Group(std::vector<GroupProperty> properties = {});

    std::shared_ptr<const Group> self() const;
    void require_parent(const Element& a, const std::string& operation) const;
    void require_same_parent(const Element& a, const Element& b, const std::string& operation) const;

private:
    std::vector<GroupProperty> properties_;
};

//=============================
// Direct Sum of Groups
//=============================
class DirectSumGroup final : public Group {
public:
    explicit DirectSumGroup(std::vector<std::shared_ptr<const Group>> factors);

    Element element(const Element::Vector& values) const override;
    Element identity() const override;
    Element inverse(const Element& a) const override;
    Element operate(const Element& a, const Element& b) const override;
    bool contains(const Element& a) const override;
    bool equals(const Element& a, const Element& b) const override;
    std::string repr(const Element& a) const override;

    std::size_t arity() const { return factors_.size(); }
    const std::shared_ptr<const Group>& factor(std::size_t i) const;

private:
    std::vector<std::shared_ptr<const Group>> factors_;
    static std::vector<GroupProperty> infer_properties(const std::vector<std::shared_ptr<const Group>>& factors);
    void require_vector_size(const Element::Vector& values, const std::string& operation) const;
};



//=============================
// Runtime Ring Interface
//=============================
class Ring : public Group {
public:
    virtual ~Ring() = default;

    virtual Element zero() const = 0;
    virtual Element one() const = 0;
    virtual Element add(const Element& a, const Element& b) const = 0;
    virtual Element neg(const Element& a) const = 0;
    virtual Element mul(const Element& a, const Element& b) const = 0;

    Element identity() const override { return zero(); }
    Element inverse(const Element& a) const override { return neg(a); }
    Element operate(const Element& a, const Element& b) const override { return add(a, b); }

    virtual Element mpower(const Element& base, long long exponent) const;
    virtual std::vector<Element> mpower(const std::vector<Element>& bases, long long exponent) const;
    virtual std::vector<Element> mpower(const std::vector<Element>& bases, const std::vector<long long>& exponents) const;

protected:
    explicit Ring(std::vector<GroupProperty> properties = {});
};



} // namespace primepy::algebra
