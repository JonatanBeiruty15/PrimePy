#include "primepy/algebra/algebraic_structures_utils.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace primepy::algebra {

Element::Element(std::shared_ptr<const Group> parent, Integer value)
    : parent_(std::move(parent)), data_(std::move(value))
{
    if (!parent_) {
        throw std::invalid_argument("Element: parent group must be non-null.");
    }
}

Element::Element(std::shared_ptr<const Group> parent, long value)
    : Element(std::move(parent), Integer(value))
{
}

Element::Element(std::shared_ptr<const Group> parent, int value)
    : Element(std::move(parent), Integer(value))
{
}

Element::Element(std::shared_ptr<const Group> parent, Vector value)
    : parent_(std::move(parent)), data_(std::move(value))
{
    if (!parent_) {
        throw std::invalid_argument("Element: parent group must be non-null.");
    }
}

bool Element::equals(const Element& other) const {
    if (parent_.get() != other.parent_.get()) return false;
    return parent_->equals(*this, other);
}

std::string Element::repr() const {
    return parent_->repr(*this);
}

Element Group::element(const Integer& /*value*/) const {
    throw std::invalid_argument("This group does not accept integer elements.");
}

Group::Group(std::vector<GroupProperty> properties)
    : properties_(std::move(properties))
{
    std::sort(properties_.begin(), properties_.end());
    properties_.erase(std::unique(properties_.begin(), properties_.end()), properties_.end());
}

bool Group::has_property(GroupProperty property) const {
    return std::find(properties_.begin(), properties_.end(), property) != properties_.end();
}

Element Group::element(const Element::Vector& /*values*/) const {
    throw std::invalid_argument("This group does not accept vector elements.");
}

bool Group::contains(const Element& a) const {
    return a.parent().get() == this;
}

Element Group::power(const Element& base, long long exponent) const {
    return GroupUtils::power(*this, base, exponent);
}

std::vector<Element> Group::power(const std::vector<Element>& bases, long long exponent) const {
    return GroupUtils::power(*this, bases, exponent);
}

std::vector<Element> Group::power(const std::vector<Element>& bases,
                                  const std::vector<long long>& exponents) const {
    return GroupUtils::power(*this, bases, exponents);
}

std::shared_ptr<const Group> Group::self() const {
    return shared_from_this();
}

void Group::require_parent(const Element& a, const std::string& operation) const {
    if (a.parent().get() != this) {
        throw std::invalid_argument(operation + ": element belongs to a different parent group.");
    }
}

void Group::require_same_parent(const Element& a, const Element& b, const std::string& operation) const {
    require_parent(a, operation);
    require_parent(b, operation);
}

DirectSumGroup::DirectSumGroup(std::vector<std::shared_ptr<const Group>> factors)
    : Group(infer_properties(factors)), factors_(std::move(factors))
{
    if (factors_.empty()) {
        throw std::invalid_argument("DirectSumGroup: at least one factor is required.");
    }
    for (const auto& factor : factors_) {
        if (!factor) {
            throw std::invalid_argument("DirectSumGroup: factor groups must be non-null.");
        }
    }
}

std::vector<GroupProperty>
DirectSumGroup::infer_properties(const std::vector<std::shared_ptr<const Group>>& factors) {
    std::vector<GroupProperty> properties;
    if (factors.empty()) {
        return properties;
    }

    const bool finite = std::all_of(factors.begin(), factors.end(), [](const auto& factor) {
        return factor && factor->has_property(GroupProperty::Finite);
    });
    const bool abelian = std::all_of(factors.begin(), factors.end(), [](const auto& factor) {
        return factor && factor->has_property(GroupProperty::Abelian);
    });

    if (finite) properties.push_back(GroupProperty::Finite);
    if (abelian) properties.push_back(GroupProperty::Abelian);
    return properties;
}

void DirectSumGroup::require_vector_size(const Element::Vector& values,
                                        const std::string& operation) const {
    if (values.size() != factors_.size()) {
        throw std::invalid_argument(
            operation + ": expected tuple of length " + std::to_string(factors_.size()) +
            ", got length " + std::to_string(values.size()) + ".");
    }
}

const std::shared_ptr<const Group>& DirectSumGroup::factor(std::size_t i) const {
    if (i >= factors_.size()) {
        throw std::out_of_range("DirectSumGroup::factor: index out of range.");
    }
    return factors_[i];
}

Element DirectSumGroup::element(const Element::Vector& values) const {
    require_vector_size(values, "DirectSumGroup::element");

    Element::Vector normalized;
    normalized.reserve(values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (values[i].parent().get() != factors_[i].get()) {
            throw std::invalid_argument("DirectSumGroup::element: component belongs to the wrong factor group.");
        }
        normalized.push_back(values[i]);
    }
    return Element(self(), std::move(normalized));
}

Element DirectSumGroup::identity() const {
    Element::Vector values;
    values.reserve(factors_.size());
    for (const auto& factor : factors_) {
        values.push_back(factor->identity());
    }
    return Element(self(), std::move(values));
}

Element DirectSumGroup::inverse(const Element& a) const {
    require_parent(a, "DirectSumGroup::inverse");
    const auto& values = std::get<Element::Vector>(a.data());
    require_vector_size(values, "DirectSumGroup::inverse");

    Element::Vector out;
    out.reserve(values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        out.push_back(factors_[i]->inverse(values[i]));
    }
    return Element(self(), std::move(out));
}

Element DirectSumGroup::operate(const Element& a, const Element& b) const {
    require_same_parent(a, b, "DirectSumGroup::operate");
    const auto& av = std::get<Element::Vector>(a.data());
    const auto& bv = std::get<Element::Vector>(b.data());
    require_vector_size(av, "DirectSumGroup::operate");
    require_vector_size(bv, "DirectSumGroup::operate");

    Element::Vector out;
    out.reserve(factors_.size());
    for (std::size_t i = 0; i < factors_.size(); ++i) {
        out.push_back(factors_[i]->operate(av[i], bv[i]));
    }
    return Element(self(), std::move(out));
}

bool DirectSumGroup::contains(const Element& a) const {
    if (a.parent().get() != this || !std::holds_alternative<Element::Vector>(a.data())) return false;
    const auto& values = std::get<Element::Vector>(a.data());
    if (values.size() != factors_.size()) return false;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!factors_[i]->contains(values[i])) return false;
    }
    return true;
}

bool DirectSumGroup::equals(const Element& a, const Element& b) const {
    require_same_parent(a, b, "DirectSumGroup::equals");
    const auto& av = std::get<Element::Vector>(a.data());
    const auto& bv = std::get<Element::Vector>(b.data());
    require_vector_size(av, "DirectSumGroup::equals");
    require_vector_size(bv, "DirectSumGroup::equals");

    for (std::size_t i = 0; i < factors_.size(); ++i) {
        if (!av[i].equals(bv[i])) return false;
    }
    return true;
}

std::string DirectSumGroup::repr(const Element& a) const {
    require_parent(a, "DirectSumGroup::repr");
    const auto& values = std::get<Element::Vector>(a.data());
    require_vector_size(values, "DirectSumGroup::repr");

    std::ostringstream out;
    out << "(";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out << ", ";
        out << values[i].repr();
    }
    if (values.size() == 1) out << ",";
    out << ")";
    return out.str();
}

Element GroupUtils::power(const Group& group, Element base, long long exponent) {
    group.require_parent(base, "GroupUtils::power");

    if (exponent < 0) {
        base = group.inverse(base);
        exponent = -exponent;
    }

    Element result = group.identity();
    while (exponent > 0) {
        if (exponent & 1LL) {
            result = group.operate(result, base);
        }
        base = group.operate(base, base);
        exponent >>= 1LL;
    }
    return result;
}

std::vector<Element> GroupUtils::power(const Group& group,
                                       const std::vector<Element>& bases,
                                       long long exponent) {
    std::vector<Element> results(bases.size(), group.identity());

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
        results[i] = power(group, bases[i], exponent);
    }
    return results;
}

std::vector<Element> GroupUtils::power(const Group& group,
                                       const std::vector<Element>& bases,
                                       const std::vector<long long>& exponents) {
    if (bases.size() != exponents.size()) {
        throw std::invalid_argument("GroupUtils::power(vec, vec): sizes must match.");
    }

    std::vector<Element> results(bases.size(), group.identity());

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
        results[i] = power(group, bases[i], exponents[i]);
    }
    return results;
}

std::shared_ptr<DirectSumGroup>
GroupUtils::direct_sum(std::vector<std::shared_ptr<const Group>> factors) {
    return std::make_shared<DirectSumGroup>(std::move(factors));
}

Ring::Ring(std::vector<GroupProperty> properties)
    : Group(std::move(properties))
{
}

Element Ring::mpower(const Element& base, long long exponent) const {
    require_parent(base, "Ring::mpower");
    if (exponent < 0) {
        throw std::invalid_argument(
            "Ring::mpower: negative exponent is not supported for general rings.");
    }

    Element result = one();
    Element factor = base;
    while (exponent > 0) {
        if (exponent & 1LL) {
            result = mul(result, factor);
        }
        exponent >>= 1LL;
        if (exponent) {
            factor = mul(factor, factor);
        }
    }
    return result;
}

std::vector<Element> Ring::mpower(const std::vector<Element>& bases, long long exponent) const {
    if (exponent < 0) {
        throw std::invalid_argument(
            "Ring::mpower: negative exponent is not supported for general rings.");
    }

    std::vector<Element> results(bases.size(), one());

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
        results[i] = mpower(bases[i], exponent);
    }
    return results;
}

std::vector<Element> Ring::mpower(const std::vector<Element>& bases,
                                  const std::vector<long long>& exponents) const {
    if (bases.size() != exponents.size()) {
        throw std::invalid_argument("Ring::mpower(vec, vec): sizes must match.");
    }

    std::vector<Element> results(bases.size(), one());

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(bases.size()); ++i) {
        results[i] = mpower(bases[i], exponents[i]);
    }
    return results;
}

} // namespace primepy::algebra
