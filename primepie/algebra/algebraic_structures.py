from __future__ import annotations
"""
primepie.algebra.algebraic_structures
=====================================

Python-side wrappers for algebraic structures:
  • Group, AdditiveModGroup
  • Ring, Integers, IntegersModRing
backed by C++ classes in `_core.algebra`.
"""

from typing import List, Sequence, Union, overload
from ..core import _core


# ==================================================================================================
# Groups
# ==================================================================================================


class Group:
    def __init__(self, backend):
        self._backend = backend

    def identity(self):
        return self._backend.identity()

    def inverse(self, a):
        return self._backend.inverse(a)

    def operate(self, a, b):
        return self._backend.operate(a, b)

    def power(self, base, exponent):
        return self._backend.power(base, exponent)


# ========================================
# Additive Group Mod n: ℤ/nℤ Python Wrapper
# ========================================
class AdditiveModGroup(Group):
    def __init__(self, modulus):
        cpp_instance = _core.algebra.AdditiveModGroup(modulus)
        super().__init__(cpp_instance)

    # --- private helpers ---
    def _power_scalar(self, base: int, exponent: int):
        return self._backend.power(int(base), int(exponent))

    def _power_batch(self, bases, exponent: int):
        return self._backend.power(list(bases), int(exponent))

    def _power_elementwise(self, bases, exponents):
        return self._backend.power(list(bases), [int(e) for e in exponents])

    # --- single public entrypoint ---
    def power(self, base_or_bases, exponent_or_exponents):
        """
        Unified power:
          - power(int base, int exponent) -> int
          - power(Iterable[int] bases, int exponent) -> list[int]
          - power(Iterable[int] bases, Iterable[int] exponents) -> list[int]
        """
        b = base_or_bases
        e = exponent_or_exponents

        # scalar: int,int
        if isinstance(b, int) and isinstance(e, int):
            return self._power_scalar(b, e)

        # batch same exponent: iterable of ints, int
        if not isinstance(b, int) and isinstance(e, int):
            return self._power_batch(b, e)

        # element-wise: iterable, iterable (must be same length; backend checks too)
        if not isinstance(b, int) and not isinstance(e, int):
            return self._power_elementwise(b, e)

        raise TypeError("power expects (int,int), (Iterable[int],int), or (Iterable[int],Iterable[int]).")
    



# ==================================================================================================
# Rings 
# ==================================================================================================

class Ring(Group):
    """
    A Ring is also an additive group in our design.
    This base wrapper forwards to the C++ virtuals.
    """
    # additive group methods come from Group (identity/inverse/operate)

    # ring primitives
    def zero(self):
        return self._backend.zero()

    def one(self):
        return self._backend.one()

    def add(self, a, b):
        return self._backend.add(a, b)

    def neg(self, a):
        return self._backend.neg(a)

    def mul(self, a, b):
        return self._backend.mul(a, b)

    # equality hook
    def is_equal(self, a, b):
        return self._backend.is_equal(a, b)





class Integers(Ring):
    """
    Python wrapper for the C++ ring of integers ℤ.

    Instance methods (ring operations):
      - zero(), one(), add(a,b), neg(a), mul(a,b), is_equal(a,b), contains(a)
      - power(...) (ADDITIVE power: repeated addition via the group law)

    Static utilities:
      - gcd(a, b)
      - is_prime(n or [n1, n2, ...])  ← unified single + batch interface
    """

    def __init__(self) -> None:
        cpp_instance = _core.algebra.Integers()
        super().__init__(cpp_instance)

    def __repr__(self) -> str:
        return "Integers(ℤ)"

    def contains(self, a: int) -> bool:
        return self._backend.contains(int(a))

    # ---------- additive power ----------
    @overload
    def power(self, base: int, exponent: int) -> int: ...
    @overload
    def power(self, bases: Sequence[int], exponent: int) -> List[int]: ...
    @overload
    def power(self, bases: Sequence[int], exponents: Sequence[int]) -> List[int]: ...

    def power(self, base_or_bases, exponent_or_exponents):
        b, e = base_or_bases, exponent_or_exponents
        if isinstance(b, int) and isinstance(e, int):
            return self._backend.power(int(b), int(e))
        if not isinstance(b, int) and isinstance(e, int):
            return self._backend.power(list(b), int(e))
        if not isinstance(b, int) and not isinstance(e, int):
            return self._backend.power(list(b), [int(x) for x in e])
        raise TypeError("power expects (int,int), (Iterable[int],int), or (Iterable[int],Iterable[int]).")

    # ---------- static helpers ----------
    @staticmethod
    def gcd(a: int, b: int) -> int:
        """Greatest common divisor of a and b."""
        return _core.algebra.Integers.gcd(int(a), int(b))

    # unified single + batch primality
    @staticmethod
    def is_prime(n_or_seq: Union[int, Sequence[int]]) -> Union[bool, List[bool]]:
        """
        Deterministic primality test.

        - is_prime(n: int) -> bool
        - is_prime([n1, n2, ...]) -> list[bool]
        """
        if isinstance(n_or_seq, int):
            return _core.algebra.Integers.is_prime(int(n_or_seq))
        if isinstance(n_or_seq, (list, tuple, set)):
            return list(_core.algebra.Integers.is_prime_array([int(x) for x in n_or_seq]))
        raise TypeError("is_prime expects int or iterable of ints")





# ========================================
# Integers modulo n: ℤ/nℤ
# ========================================


class IntegersModRing(Ring):
    def __init__(self, modulus: int):
        cpp_instance = _core.algebra.IntegersModRing(modulus)
        super().__init__(cpp_instance)

    def contains(self, a: int) -> bool:
        return self._backend.contains(a)

    @property
    def modulus(self) -> int:
        return self._backend.modulus

    # --- multiplicative helpers (call backend .mpower) ---
    def _mpower_scalar(self, base: int, exponent: int):
        return self._backend.mpower(int(base), int(exponent))

    def _mpower_batch(self, bases, exponent: int):
        return self._backend.mpower(list(bases), int(exponent))

    def _mpower_elementwise(self, bases, exponents):
        return self._backend.mpower(list(bases), [int(e) for e in exponents])

    # --- single public entrypoint for multiplicative power ---
    def mpower(self, base_or_bases, exponent_or_exponents):
        """
        Multiplicative power on Z/nZ:
          - mpower(int base, int exponent) -> int
          - mpower(Iterable[int] bases, int exponent) -> list[int]
          - mpower(Iterable[int] bases, Iterable[int] exponents) -> list[int]

        Note: negative exponents are not supported for general rings (backend will raise).
        """
        b = base_or_bases
        e = exponent_or_exponents

        if isinstance(b, int) and isinstance(e, int):
            return self._mpower_scalar(b, e)
        if not isinstance(b, int) and isinstance(e, int):
            return self._mpower_batch(b, e)
        if not isinstance(b, int) and not isinstance(e, int):
            return self._mpower_elementwise(b, e)

        raise TypeError("mpower expects (int,int), (Iterable[int],int), or (Iterable[int],Iterable[int]).")












if __name__ == "__main__":
    # Groups (Z/7Z, +)
    G = AdditiveModGroup(7)
    print("Additive group Z/7Z")
    print("identity:", G.identity())                 # 0
    print("inverse(3):", G.inverse(3))               # 4
    print("operate(3,6):", G.operate(3, 6))          # 2
    print("power(2,3):", G.power(2, 3))              # 6
    print("power([1,2,3], 2):", G.power([1, 2, 3], 2))               # [2, 4, 6]
    print("power([1,2,3],[0,1,2]):", G.power([1, 2, 3], [0, 1, 2]))  # [0, 2, 6]

    # Ring Z/7Z
    R = IntegersModRing(7)
    print("\nRing Z/7Z")
    print("zero, one:", R.zero(), R.one())           # 0 1
    print("add(5,6):", R.add(5, 6))                  # 4
    print("neg(3):", R.neg(3))                       # 4
    print("mul(3,5):", R.mul(3, 5))                  # 1
    print("is_equal(10,3):", R.is_equal(10, 3))      # True

    # ADDITIVE power on the ring (inherited from Ring.power)
    print("additive power R.power(3,4):", R.power(3, 4))  # 3+3+3+3 ≡ 5

    # MULTIPLICATIVE power on the ring (IntegersModRing.mpower)
    print("multiplicative R.mpower(3,4):", R.mpower(3, 4))                # 4
    print("multiplicative R.mpower([1,2,3,6], 3):", R.mpower([1,2,3,6], 3))   # [1,1,6,6]
    print("multiplicative R.mpower([2,3,4,5],[0,1,2,3]):",
          R.mpower([2,3,4,5], [0,1,2,3]))                                 # [1,3,2,6]
    print("(2 + 3) * 4:", R.mul(R.add(2, 3), 4))   # (2+3)*4 ≡ 20 ≡ 6 mod 7




    print("\n=== Integers.is_prime (unified scalar + batch) ===")

    # scalar checks
    assert Integers.is_prime(2) is True
    assert Integers.is_prime(3) is True
    assert Integers.is_prime(4) is False
    assert Integers.is_prime(1) is False
    assert Integers.is_prime(0) is False
    assert Integers.is_prime(-7) is False
    assert Integers.is_prime(97) is True
    assert Integers.is_prime(10007) is True
    assert Integers.is_prime(1_000_000_007) is True  # large 32-bit prime

    # batch check (order preserved)
    nums = [2, 4, 5, 9, 1, 97, 1_000_000_007, 0, -11, 10007]
    expected = [True, False, True, False, False, True, True, False, False, True]
    out = Integers.is_prime(nums)
    print("batch:", out)
    assert out == expected, f"expected {expected}, got {out}"

    # quick mixed sanity
    assert Integers.is_prime([2, 3, 10, 11, 12]) == [True, True, False, True, False]

    # (optional) error cases: wrong types should raise
    try:
        Integers.is_prime("13")  # type: ignore
        raise AssertionError("Expected TypeError for string input")
    except TypeError:
        pass