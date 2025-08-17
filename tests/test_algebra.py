# tests/algebra_tests.py

import sys
import os
import pytest

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from primepy.algebra.algebraic_structures import AdditiveModGroup

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
    result = group_mod7.power_batch([1, 2, 3], 2)
    assert result == [2, 4, 6]

def test_power_elementwise(group_mod7):
    result = group_mod7.power_elementwise([1, 2, 3], [0, 1, 2])
    assert result == [0, 2, 6]