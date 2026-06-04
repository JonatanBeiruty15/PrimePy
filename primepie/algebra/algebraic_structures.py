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

Element = _core.algebra.Element
GroupProperty = _core.algebra.GroupProperty


# ==================================================================================================
# Groups
# ==================================================================================================


class Group:
    def __init__(self, backend):
        self._backend = backend

    def __call__(self, value):
        return self.element(value)

    def element(self, value):
        return self._backend.element(value)

    @property
    def properties(self):
        return list(self._backend.properties)

    def has_property(self, property_):
        return self._backend.has_property(property_)

    def identity(self):
        return self._backend.identity()

    def inverse(self, a):
        return self._backend.inverse(a)

    def operate(self, a, b):
        return self._backend.operate(a, b)

    def contains(self, a):
        return self._backend.contains(a)

    def power(self, base, exponent):
        if isinstance(base, (list, tuple)) and not isinstance(base, Element):
            if isinstance(exponent, int):
                if hasattr(self._backend, "power_many"):
                    return self._backend.power_many(list(base), int(exponent))
                return self._backend.power(list(base), int(exponent))
            if hasattr(self._backend, "power_many"):
                return self._backend.power_many(list(base), [int(e) for e in exponent])
            return self._backend.power(list(base), [int(e) for e in exponent])
        return self._backend.power(base, int(exponent))

    def power_many(self, bases, exponent):
        if isinstance(exponent, int):
            if hasattr(self._backend, "power_many"):
                return self._backend.power_many(list(bases), int(exponent))
            return self._backend.power(list(bases), int(exponent))
        if hasattr(self._backend, "power_many"):
            return self._backend.power_many(list(bases), [int(e) for e in exponent])
        return self._backend.power(list(bases), [int(e) for e in exponent])


# ========================================
# Additive Group Mod n: ℤ/nℤ Python Wrapper
# ========================================
class AdditiveModGroup(Group):
    def __init__(self, modulus):
        cpp_instance = _core.algebra.AdditiveModGroup(modulus)
        super().__init__(cpp_instance)

    @property
    def modulus(self) -> int:
        return self._backend.modulus


class MultiplicativeModGroup(Group):
    def __init__(self, modulus):
        cpp_instance = _core.algebra.MultiplicativeModGroup(modulus)
        super().__init__(cpp_instance)

    @property
    def modulus(self) -> int:
        return self._backend.modulus

    def contains(self, a) -> bool:
        return self._backend.contains(a)


class DirectSumGroup(Group):
    def __init__(self, groups):
        self.groups = list(groups)
        cpp_instance = _core.algebra.DirectSumGroup([g._backend for g in self.groups])
        super().__init__(cpp_instance)

    @property
    def arity(self) -> int:
        return self._backend.arity

    def power(self, base, exponent):
        if isinstance(base, (list, tuple)) and not isinstance(base, Element):
            is_batch = len(base) != self.arity
            if is_batch:
                if isinstance(exponent, int):
                    return self._backend.power_many(list(base), int(exponent))
                return self._backend.power_many(list(base), [int(e) for e in exponent])
        return self._backend.power(base, int(exponent))
    



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

    def mpower(self, base, exponent):
        if isinstance(base, (list, tuple)) and not isinstance(base, Element):
            if isinstance(exponent, int):
                if hasattr(self._backend, "mpower_many"):
                    return self._backend.mpower_many(list(base), int(exponent))
                return self._backend.mpower(list(base), int(exponent))
            if hasattr(self._backend, "mpower_many"):
                return self._backend.mpower_many(list(base), [int(e) for e in exponent])
            return self._backend.mpower(list(base), [int(e) for e in exponent])
        return self._backend.mpower(base, int(exponent))


class Field(Ring):
    def reciprocal(self, a):
        return self._backend.reciprocal(a)

    def div(self, a, b):
        return self._backend.div(a, b)

    @property
    def characteristic(self) -> int:
        return self._backend.characteristic




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
            return self._backend.power_many(list(b), int(e))
        if not isinstance(b, int) and not isinstance(e, int):
            return self._backend.power_many(list(b), [int(x) for x in e])
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


class RationalField(Field):
    def __init__(self):
        cpp_instance = _core.algebra.RationalField()
        super().__init__(cpp_instance)

    def __repr__(self) -> str:
        return "RationalField(ℚ)"


class FiniteField(Field):
    def __init__(self, prime: int, degree: int = 1):
        cpp_instance = _core.algebra.FiniteField(prime, degree)
        super().__init__(cpp_instance)

    @property
    def modulus(self) -> int:
        return self._backend.modulus

    @property
    def degree(self) -> int:
        return self._backend.degree

    def __repr__(self) -> str:
        if self.degree == 1:
            return f"FiniteField({self.modulus})"
        return f"FiniteField({self.modulus}, degree={self.degree})"


class PolynomialRing(Ring):
    def __init__(self, coefficient_ring: Ring, variable: str = "X", use_parentheses: bool = True):
        if not hasattr(coefficient_ring, "_backend"):
            raise TypeError("coefficient_ring must be a PrimePie ring.")
        if not isinstance(variable, str) or len(variable) != 1:
            raise ValueError("variable must be exactly one character.")

        self.coefficient_ring = coefficient_ring
        cpp_instance = _core.algebra.PolynomialRing(
            coefficient_ring._backend,
            variable,
            bool(use_parentheses),
        )
        super().__init__(cpp_instance)

    @property
    def variable(self) -> str:
        return self._backend.variable

    def element(self, value):
        if isinstance(value, int) or isinstance(value, Element):
            return self._backend.element([value])
        return self._backend.element(value)

    def power(self, base, exponent):
        return self._backend.power(base, int(exponent))

    def mpower(self, base, exponent):
        return self._backend.mpower(base, int(exponent))









