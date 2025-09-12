# tests/algebra_tests.py

import sys
import os
import pytest

# ensure project root (folder containing 'primepie') is on sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from primepie.algebra.algebraic_structures import AdditiveModGroup, IntegersModRing

# ======================
# Groups: ℤ/7ℤ (additive)
# ======================

@pytest.fixture
def group_mod7():
    return AdditiveModGroup(7)

def test_identity(group_mod7):
    assert group_mod7.identity() == 0

def test_inverse(group_mod7):
    assert group_mod7.inverse(3) == 4
    assert group_mod7.inverse(6) == 1

def test_operate(group_mod7):
    assert group_mod7.operate(3, 6) == 2
    assert group_mod7.operate(2, 2) == 4

def test_power(group_mod7):
    assert group_mod7.power(2, 3) == 6
    assert group_mod7.power(3, 2) == 6

def test_power_batch(group_mod7):
    result = group_mod7.power([1, 2, 3], 2)  # unified API
    assert result == [2, 4, 6]

def test_power_elementwise(group_mod7):
    result = group_mod7.power([1, 2, 3], [0, 1, 2])  # unified API
    assert result == [0, 2, 6]


# ======================
# Rings: ℤ/nℤ (integers mod n)
# ======================

@pytest.fixture
def ring_mod7():
    return IntegersModRing(7)

@pytest.fixture
def ring_mod1():
    # edge case: zero ring
    return IntegersModRing(1)

def test_ring_primitives_mod7(ring_mod7):
    R = ring_mod7
    assert R.zero() == 0
    assert R.one() == 1
    assert R.add(5, 6) == 4       # 11 ≡ 4 mod 7
    assert R.neg(3) == 4          # -3 ≡ 4 mod 7
    assert R.mul(3, 5) == 1       # 15 ≡ 1 mod 7
    assert R.is_equal(10, 3) is True
    assert R.contains(12345) is True
    assert R.modulus == 7

def test_ring_power_scalar_mod7(ring_mod7):
    R = ring_mod7
    assert R.power(3, 0) == 1
    assert R.power(3, 1) == 3
    assert R.power(3, 2) == 2     # 9 ≡ 2
    assert R.power(3, 4) == 4     # 81 ≡ 4

def test_ring_power_batch_same_exponent_mod7(ring_mod7):
    R = ring_mod7
    out = R.power([1, 2, 3, 6], 3)
    assert out == [1, 1, 6, 6]

def test_ring_power_elementwise_mod7(ring_mod7):
    R = ring_mod7
    out = R.power([2, 3, 4, 5], [0, 1, 2, 3])
    assert out == [1, 3, 2, 6]

def test_ring_distributivity_spot_checks(ring_mod7):
    R = ring_mod7
    a, b, c = 3, 5, 6
    lhs1 = R.mul(a, R.add(b, c))
    rhs1 = R.add(R.mul(a, b), R.mul(a, c))
    assert lhs1 == rhs1

    lhs2 = R.mul(R.add(a, b), c)
    rhs2 = R.add(R.mul(a, c), R.mul(b, c))
    assert lhs2 == rhs2

def test_ring_zero_ring_mod1(ring_mod1):
    R = ring_mod1
    assert R.zero() == 0
    assert R.one() == 0           # 0 = 1 in ℤ/1ℤ
    assert R.add(123, 456) == 0
    assert R.mul(123, 456) == 0
    assert R.power(5, 123456789) == 0
    assert R.is_equal(7, 0) is True
    assert R.modulus == 1

def test_ring_negative_exponent_raises(ring_mod7):
    R = ring_mod7
    # pybind11 maps std::invalid_argument to ValueError by default
    with pytest.raises(ValueError):
        _ = R.power(3, -1)