# tests/algebra_tests.py

import sys
import os
import pytest

# -----------------------------------------------------------------------------------
# Ensure project root (folder containing 'primepie') is on sys.path for direct pytest
# -----------------------------------------------------------------------------------
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from primepie.algebra.algebraic_structures import AdditiveModGroup, IntegersModRing, Integers, PolynomialsOverIntegers


# ===================================================================================
# SECTION 1 — GROUP TESTS: Additive Group ℤ/7ℤ
# ===================================================================================

@pytest.fixture
def group_mod7():
    """Fixture: additive group of integers mod 7 (Z/7Z, +)."""
    return AdditiveModGroup(7)

def test_identity(group_mod7):
    """The identity element under addition mod 7 is 0."""
    assert group_mod7.identity() == 0

def test_inverse(group_mod7):
    """Every element a has an inverse b such that a + b ≡ 0 mod 7."""
    assert group_mod7.inverse(3) == 4
    assert group_mod7.inverse(6) == 1

def test_operate(group_mod7):
    """Group operation corresponds to modular addition."""
    assert group_mod7.operate(3, 6) == 2   # (3+6) % 7 = 2
    assert group_mod7.operate(2, 2) == 4

def test_power(group_mod7):
    """Additive power = repeated addition (e.g. 2 added 3 times = 6 mod 7)."""
    assert group_mod7.power(2, 3) == 6
    assert group_mod7.power(3, 2) == 6

def test_power_batch(group_mod7):
    """Batch power: apply same exponent to multiple bases."""
    result = group_mod7.power([1, 2, 3], 2)
    assert result == [2, 4, 6]

def test_power_elementwise(group_mod7):
    """Element-wise power: pairwise (b_i)^(e_i) in additive sense."""
    result = group_mod7.power([1, 2, 3], [0, 1, 2])
    assert result == [0, 2, 6]


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
    assert R.zero() == 0
    assert R.one() == 1
    assert R.add(5, 6) == 4       # 11 ≡ 4 mod 7
    assert R.neg(3) == 4          # -3 ≡ 4 mod 7
    assert R.mul(3, 5) == 1       # 15 ≡ 1 mod 7
    assert R.is_equal(10, 3) is True
    assert R.contains(12345) is True
    assert R.modulus == 7

def test_ring_mpower_scalar_mod7(ring_mod7):
    """Multiplicative power (modular exponentiation)."""
    R = ring_mod7
    assert R.mpower(3, 0) == 1           # a^0 = 1
    assert R.mpower(3, 1) == 3
    assert R.mpower(3, 2) == 2           # 9 ≡ 2
    assert R.mpower(3, 4) == 4           # 81 ≡ 4

def test_ring_mpower_batch_same_exponent_mod7(ring_mod7):
    """Batch modular exponentiation: same exponent for several bases."""
    R = ring_mod7
    out = R.mpower([1, 2, 3, 6], 3)
    assert out == [1, 1, 6, 6]

def test_ring_mpower_elementwise_mod7(ring_mod7):
    """Element-wise modular exponentiation."""
    R = ring_mod7
    out = R.mpower([2, 3, 4, 5], [0, 1, 2, 3])
    assert out == [1, 3, 2, 6]

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
    assert R.mul(R.add(2, 3), 4) == 6

def test_ring_zero_ring_mod1(ring_mod1):
    """Edge case: ℤ/1ℤ is the trivial ring (everything = 0)."""
    R = ring_mod1
    assert R.zero() == 0
    assert R.one() == 0           # 0 = 1 in the zero ring
    assert R.add(123, 456) == 0
    assert R.mul(123, 456) == 0
    assert R.mpower(5, 123456789) == 0
    assert R.power(5, 123456789) == 0
    assert R.is_equal(7, 0) is True
    assert R.modulus == 1

def test_ring_negative_exponent_raises_for_mpower(ring_mod7):
    """mpower should raise ValueError for negative exponents."""
    R = ring_mod7
    with pytest.raises(ValueError):
        _ = R.mpower(3, -1)


# ===================================================================================
# SECTION 3 — RING TESTS: Integers ℤ
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





# ===================================================================================
# SECTION 3 — RING TESTS: Polynomials over Z ;Z[X]
# ===================================================================================



@pytest.fixture
def R():
    """The polynomial ring Z[x]."""
    return PolynomialsOverIntegers()


# -----------------------------
# Basics: zero/one/degree/zero?
# -----------------------------
def test_poly_basics(R):
    z = R.zero()
    o = R.one()
    assert z == []                 # canonical zero
    assert o == [1]                # constant 1
    assert R.degree(z) == -1
    assert R.degree(o) == 0
    assert R.is_zero(z) is True
    assert R.is_zero(o) is False

    # normalization trims trailing zeros
    assert R.normalize([3, 0, 0]) == [3]
    assert R.normalize([0, 0, 0]) == []


# -----------------------------
# Add / Neg / Sub
# -----------------------------
def test_poly_add_neg_sub(R):
    a = [1, 2, 3]        # 1 + 2x + 3x^2
    b = [4, -2]          # 4 - 2x
    assert R.add(a, b) == [5, 0, 3]       # (1+4) + (2-2)x + 3x^2
    assert R.neg(a) == [-1, -2, -3]
    assert R.sub(a, b) == [-3, 4, 3]


# -----------------------------
# to_string
# -----------------------------
def test_poly_to_string(R):
    cases = [
        ([], "0"),
        ([1], "1"),
        ([-1], "-1"),
        ([0, 1], "x"),
        ([0, -1], "-x"),
        ([3, 0, 2], "3 + 2x^2"),
        ([-3, 1, -1, 0, 5], "-3 + x - x^2 + 5x^4"),
    ]
    for coeffs, want in cases:
        assert R.to_string(coeffs) == want


# -----------------------------
# Multiplication (naive)
# -----------------------------
def test_poly_mul_naive_small(R):
    a = [1, 2, 3]    # 1 + 2x + 3x^2
    b = [4, 5]       # 4 + 5x
    # Expected: 4 + 13x + 22x^2 + 15x^3
    assert R.mul(a, b, method="naive") == [4, 13, 22, 15]

    # with trailing zeros in inputs -> trimmed output
    assert R.mul([1, 0, 0], [0, 0, 2], method="naive") == [0, 0, 2]


# -----------------------------
# Karatsuba vs Naive agreement
# -----------------------------
def test_poly_mul_methods_agree(R):
    # random-ish shapes that trigger deeper recursion
    a = [1, -2, 3, 0, 0, 4, 5, 0, -1]
    b = [2, 0, 0, -3, 6, 1, 0, 0, 7, -5]
    prod_naive = R.mul(a, b, method="naive")
    prod_karat = R.mul(a, b, method="karatsuba")
    assert prod_naive == prod_karat


# -----------------------------
# Distributivity: a*(b+c) = ab + ac
# -----------------------------
def test_poly_distributivity(R):
    a = [1, 2, 1]         # 1 + 2x + x^2
    b = [2, -1, 3]        # 2 - x + 3x^2
    c = [0, 5]            # 5x
    # Check both algorithms
    for method in ("naive", "karatsuba"):
        lhs = R.mul(a, R.add(b, c), method=method)
        rhs = R.add(R.mul(a, b, method=method), R.mul(a, c, method=method))
        assert lhs == rhs


# -----------------------------
# Identity & annihilator checks
# -----------------------------
def test_poly_identity_annihilator(R):
    a = [3, 0, -2, 1]   # 3 - 2x^2 + x^3
    z = R.zero()
    o = R.one()
    assert R.mul(a, o, method="naive") == a
    assert R.mul(o, a, method="karatsuba") == a
    assert R.mul(a, z, method="naive") == []
    assert R.mul(z, a, method="karatsuba") == []


# -----------------------------
# Method property and auto
# -----------------------------
def test_poly_method_property_and_auto(R):
    # default method is whatever you set in wrapper; we just ensure API works
    a = [1, 1, 1, 1, 1, 1, 1]   # degree 6
    b = [1, 2, 3, 4, 5, 6, 7]
    # flip methods and ensure consistent results
    R.method = "naive"
    p1 = R.mul(a, b)  # uses current method
    R.method = "karatsuba"
    p2 = R.mul(a, b)
    R.method = "auto"
    p3 = R.mul(a, b)
    assert p1 == p2 == p3