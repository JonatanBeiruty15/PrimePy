# tests/algebra_tests.py

import sys
import os
import pytest

# -----------------------------------------------------------------------------------
# Ensure project root (folder containing 'primepie') is on sys.path for direct pytest
# -----------------------------------------------------------------------------------
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from primepie.algebra.algebraic_structures import (
    AdditiveModGroup,
    GroupProperty,
    MultiplicativeModGroup,
    DirectSumGroup,
    IntegersModRing,
    Integers,
    PolynomialRing,
)


# ===================================================================================
# SECTION 1 — GROUP TESTS: Additive Group ℤ/7ℤ
# ===================================================================================

@pytest.fixture
def group_mod7():
    """Fixture: additive group of integers mod 7 (Z/7Z, +)."""
    return AdditiveModGroup(7)

def test_identity(group_mod7):
    """The identity element under addition mod 7 is 0."""
    assert group_mod7.identity().value == 0

def test_inverse(group_mod7):
    """Every element a has an inverse b such that a + b ≡ 0 mod 7."""
    assert group_mod7.inverse(group_mod7(3)).value == 4
    assert group_mod7.inverse(6).value == 1

def test_operate(group_mod7):
    """Group operation corresponds to modular addition."""
    assert group_mod7.operate(group_mod7(3), group_mod7(6)).value == 2   # (3+6) % 7 = 2
    assert group_mod7.operate(2, 2).value == 4

def test_power(group_mod7):
    """Additive power = repeated addition (e.g. 2 added 3 times = 6 mod 7)."""
    assert group_mod7.power(2, 3).value == 6
    assert group_mod7.power(3, 2).value == 6

def test_power_batch(group_mod7):
    """Batch power: apply same exponent to multiple bases."""
    result = group_mod7.power([1, 2, 3], 4)
    assert [x.value for x in result] == [4, 1, 5]

def test_power_elementwise(group_mod7):
    """Element-wise power: pairwise (b_i)^(e_i) in additive sense."""
    result = group_mod7.power([1, 2, 3], [0, 1, 2])
    assert [x.value for x in result] == [0, 2, 6]


def test_element_creation_and_equality(group_mod7):
    assert group_mod7(10).value == 3
    assert group_mod7(3) == group_mod7(10)
    assert group_mod7.has_property(GroupProperty.Finite)
    assert group_mod7.has_property(GroupProperty.Abelian)


def test_element_printing_in_python(group_mod7, capsys):
    """Python print/repr should use the C++ group-specific representation."""
    element = group_mod7(10)
    print(element)

    captured = capsys.readouterr()
    assert captured.out.strip() == "3 mod 7"
    assert repr(element) == "3 mod 7"


def test_multiplicative_mod_group_runtime():
    U7 = MultiplicativeModGroup(7)
    assert U7.identity().value == 1
    assert U7(3).value == 3
    assert U7.inverse(3).value == 5
    assert U7.operate(3, 5).value == 1
    assert U7.power(3, 4).value == 4
    assert U7.power(3, -1).value == 5
    with pytest.raises(ValueError):
        MultiplicativeModGroup(8)(2)


def test_direct_sum_group_runtime():
    Z5 = AdditiveModGroup(5)
    Z7 = AdditiveModGroup(7)
    G = DirectSumGroup([Z5, Z7])
    assert G.has_property(GroupProperty.Finite)
    assert G.has_property(GroupProperty.Abelian)

    a = G((2, 3))
    b = G((4, 6))
    assert a.value == (2, 3)
    assert G.identity().value == (0, 0)
    assert G.operate(a, b).value == (1, 2)
    assert G.inverse(a).value == (3, 4)
    assert G.power(a, 3).value == (1, 2)


def test_direct_sum_element_printing_in_python(capsys):
    """Direct sum elements should print using each component group's repr."""
    Z5 = AdditiveModGroup(5)
    Z7 = AdditiveModGroup(7)
    G = DirectSumGroup([Z5, Z7])

    element = G((7, 10))
    print(element)

    captured = capsys.readouterr()
    assert captured.out.strip() == "(2 mod 5, 3 mod 7)"
    assert repr(element) == "(2 mod 5, 3 mod 7)"


def test_nested_direct_sum_group_runtime():
    Z5 = AdditiveModGroup(5)
    Z7 = AdditiveModGroup(7)
    G = DirectSumGroup([Z5, Z7])
    H = DirectSumGroup([G, Z5, Z7])

    a = H(((1, 2), 3, 4))
    b = H(((4, 6), 4, 5))
    assert H.operate(a, b).value == ((0, 1), 2, 2)


def test_nested_direct_sum_element_printing_in_python(capsys):
    """Nested direct sums should preserve the nested tuple shape when printed."""
    Z5 = AdditiveModGroup(5)
    Z7 = AdditiveModGroup(7)
    G = DirectSumGroup([Z5, Z7])
    H = DirectSumGroup([G, Z5, Z7])

    element = H(((7, 10), 8, 11))
    print(element)

    captured = capsys.readouterr()
    assert captured.out.strip() == "((2 mod 5, 3 mod 7), 3 mod 5, 4 mod 7)"
    assert repr(element) == "((2 mod 5, 3 mod 7), 3 mod 5, 4 mod 7)"


def test_direct_sum_validation_errors():
    Z5 = AdditiveModGroup(5)
    Z7 = AdditiveModGroup(7)
    G = DirectSumGroup([Z5, Z7])

    with pytest.raises(ValueError):
        G((1,))
    with pytest.raises(ValueError):
        G((Z7(1), Z5(1)))
    with pytest.raises(ValueError):
        Z5.operate(Z5(1), Z7(1))


# ===================================================================================
# SECTION 2 — RING TESTS: Integers Mod n (ℤ/nℤ)
# ===================================================================================

@pytest.fixture
def ring_mod7():
    """Fixture: ring of integers modulo 7 (ℤ/7ℤ)."""
    return IntegersModRing(7)

@pytest.fixture
def ring_mod1():
    """Fixture: zero ring ℤ/1ℤ, where every element ≡ 0."""
    return IntegersModRing(1)

def test_ring_primitives_mod7(ring_mod7):
    """Basic ring properties and primitive operations."""
    R = ring_mod7
    assert R.zero().value == 0
    assert R.one().value == 1
    assert R(10).value == 3
    assert R.add(5, 6).value == 4       # 11 ≡ 4 mod 7
    assert R.neg(3).value == 4          # -3 ≡ 4 mod 7
    assert R.mul(3, 5).value == 1       # 15 ≡ 1 mod 7
    assert R.is_equal(10, 3) is True
    assert R.contains(12345) is True
    assert R.modulus == 7
    assert R.has_property(GroupProperty.Finite)
    assert R.has_property(GroupProperty.Abelian)

def test_ring_mpower_scalar_mod7(ring_mod7):
    """Multiplicative power (modular exponentiation)."""
    R = ring_mod7
    assert R.mpower(3, 0).value == 1           # a^0 = 1
    assert R.mpower(3, 1).value == 3
    assert R.mpower(3, 2).value == 2           # 9 ≡ 2
    assert R.mpower(3, 4).value == 4           # 81 ≡ 4

def test_ring_mpower_batch_same_exponent_mod7(ring_mod7):
    """Batch modular exponentiation: same exponent for several bases."""
    R = ring_mod7
    out = R.mpower([1, 2, 3, 6], 3)
    assert [x.value for x in out] == [1, 1, 6, 6]

def test_ring_mpower_elementwise_mod7(ring_mod7):
    """Element-wise modular exponentiation."""
    R = ring_mod7
    out = R.mpower([2, 3, 4, 5], [0, 1, 2, 3])
    assert [x.value for x in out] == [1, 3, 2, 6]

def test_ring_distributivity_spot_checks(ring_mod7):
    """Check distributive property: a*(b+c) = a*b + a*c (mod 7)."""
    R = ring_mod7
    a, b, c = 3, 5, 6
    lhs1 = R.mul(a, R.add(b, c))
    rhs1 = R.add(R.mul(a, b), R.mul(a, c))
    assert lhs1 == rhs1

    lhs2 = R.mul(R.add(a, b), c)
    rhs2 = R.add(R.mul(a, c), R.mul(b, c))
    assert lhs2 == rhs2

def test_ring_add_mul_combination_example(ring_mod7):
    """Composite check: (2 + 3) * 4 ≡ 20 ≡ 6 (mod 7)."""
    R = ring_mod7
    assert R.mul(R.add(2, 3), 4).value == 6

def test_ring_zero_ring_mod1(ring_mod1):
    """Edge case: ℤ/1ℤ is the trivial ring (everything = 0)."""
    R = ring_mod1
    assert R.zero().value == 0
    assert R.one().value == 0           # 0 = 1 in the zero ring
    assert R.add(123, 456).value == 0
    assert R.mul(123, 456).value == 0
    assert R.mpower(5, 123456789).value == 0
    assert R.power(5, 123456789).value == 0
    assert R.is_equal(7, 0) is True
    assert R.modulus == 1

def test_ring_negative_exponent_raises_for_mpower(ring_mod7):
    """mpower should raise ValueError for negative exponents."""
    R = ring_mod7
    with pytest.raises(ValueError):
        _ = R.mpower(3, -1)


# ===================================================================================
# SECTION 3 — RING TESTS: Polynomial rings R[X]
# ===================================================================================

def test_polynomial_ring_basic_operations():
    R = IntegersModRing(5)
    P = PolynomialRing(R)

    assert P.has_property(GroupProperty.Abelian)
    assert not P.has_property(GroupProperty.Finite)
    assert P.variable == "X"
    assert P.zero().value == ()
    assert P.one().value == (1,)

    f = P([1, 2])       # 1 + 2X
    g = P([4, 1])       # 4 + X
    assert f.value == (1, 2)
    assert P([1, 2, 0, 0]).value == (1, 2)
    assert P.is_equal(f, P([1, 2, 0, 0]))

    assert P.add(f, g).value == (0, 3)
    assert P.neg(f).value == (4, 3)
    assert P.mul(f, g).value == (4, 4, 2)


def test_polynomial_ring_scalar_coercion():
    R = IntegersModRing(5)
    P = PolynomialRing(R)

    f = P([1, 2])
    scalar = R(3)
    constant_scalar = P([scalar])

    assert P.contains(scalar)
    assert P.contains(3)
    assert P.is_equal(scalar, constant_scalar)
    assert scalar != constant_scalar

    assert P(3).value == (3,)
    assert P.add(f, scalar).value == (4, 2)
    assert P.add(f, 3).value == (4, 2)
    assert P.mul(scalar, f).value == (3, 1)
    assert P.mul(3, f).value == (3, 1)
    assert P.is_equal(R.zero(), P.zero())
    assert P.add(P.zero(), R.zero()).value == ()


def test_polynomial_ring_printing_options():
    R = IntegersModRing(5)
    P = PolynomialRing(R)
    f = P([2, 3, 1])

    assert repr(f) == "(2 mod 5) + (3 mod 5) * X + (1 mod 5) * X^2"
    assert repr(P.zero()) == "0"
    assert repr(P(3)) == "(3 mod 5)"

    T = PolynomialRing(R, variable="t", use_parentheses=False)
    assert repr(T([2, 3, 1])) == "2 mod 5 + 3 mod 5 * t + 1 mod 5 * t^2"


def test_polynomial_ring_validation_errors():
    R5 = IntegersModRing(5)
    R7 = IntegersModRing(7)
    P = PolynomialRing(R5)

    assert not P.contains(R7(1))
    with pytest.raises(ValueError):
        P([R7(1)])
    with pytest.raises(ValueError):
        PolynomialRing(R5, variable="XY")


# ===================================================================================
# SECTION 4 — RING TESTS: Integers ℤ
# ===================================================================================

def test_integers_gcd_basics():
    """Check fundamental properties of gcd over integers."""
    assert Integers.gcd(84, 30) == 6
    assert Integers.gcd(30, 84) == 6           # symmetry
    assert Integers.gcd(7, 13) == 1            # coprime numbers
    assert Integers.gcd(0, 5) == 5
    assert Integers.gcd(5, 0) == 5
    assert Integers.gcd(0, 0) == 0             # by convention
    assert Integers.gcd(-42, 56) == 14         # sign-insensitive
    assert Integers.gcd(42, -56) == 14
    assert Integers.gcd(-42, -56) == 14

def test_integers_gcd_more_cases():
    """Extra gcd edge and random cases."""
    assert Integers.gcd(12, 36) == 12
    assert Integers.gcd(36, 12) == 12
    assert Integers.gcd(101*97, 101*131) == 101  # common factor 101
    assert Integers.gcd(99991, 99989) == 1       # consecutive primes

def test_integers_is_prime_scalar():
    """Deterministic primality for single integers (scalar form)."""
    assert Integers.is_prime(2) is True
    assert Integers.is_prime(3) is True
    assert Integers.is_prime(4) is False
    assert Integers.is_prime(1) is False
    assert Integers.is_prime(0) is False
    assert Integers.is_prime(-7) is False
    assert Integers.is_prime(97) is True
    assert Integers.is_prime(10007) is True
    assert Integers.is_prime(1_000_000_007) is True      # large 32-bit prime
    assert Integers.is_prime(1_000_000_008) is False     # adjacent composite
    assert Integers.is_prime(2305843009213693951) is True  # 2^61 - 1 (Mersenne prime)

def test_integers_is_prime_batch():
    """Unified batch primality: vectorized deterministic check."""
    nums = [2, 4, 5, 9, 1, 97, 1_000_000_007, 0, -11, 10007]
    expected = [True, False, True, False, False, True, True, False, False, True]
    out = Integers.is_prime(nums)
    assert out == expected

def test_integers_is_prime_type_errors():
    """is_prime should reject unsupported input types (e.g. strings)."""
    with pytest.raises(TypeError):
        Integers.is_prime("13")   # type: ignore
