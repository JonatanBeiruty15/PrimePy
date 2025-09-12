from ..core import _core


# ======================================================
# Groups
# ======================================================


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
    



# ======================================================
# Rings 
# ======================================================

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


# ========================================
# Integers modulo n: ℤ/nℤ (wrapper)
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

        Note: negative exponents are not supported for general rings (backend will raise).
        """
        b = base_or_bases
        e = exponent_or_exponents

        if isinstance(b, int) and isinstance(e, int):
            return self._power_scalar(b, e)

        if not isinstance(b, int) and isinstance(e, int):
            return self._power_batch(b, e)

        if not isinstance(b, int) and not isinstance(e, int):
            return self._power_elementwise(b, e)

        raise TypeError("power expects (int,int), (Iterable[int],int), or (Iterable[int],Iterable[int]).")





if __name__ == "__main__":
    # ======================
    # Groups (ℤ/7ℤ, additive)
    # ======================
    G = AdditiveModGroup(7)
    print("Additive group Z/7Z")
    print("identity:", G.identity())                 # 0
    print("inverse(3):", G.inverse(3))               # 4
    print("operate(3,6):", G.operate(3, 6))          # 2

    # unified power
    print("power(2,3):", G.power(2, 3))              # 6
    print("power([1,2,3], 2):", G.power([1, 2, 3], 2))               # [2, 4, 6]
    print("power([1,2,3],[0,1,2]):", G.power([1, 2, 3], [0, 1, 2]))  # [0, 2, 6]

    # ==========
    # Rings ℤ/7ℤ
    # ==========
    R = IntegersModRing(7)
    print("\nRing Z/7Z")
    print("zero, one:", R.zero(), R.one())           # 0 1
    print("add(5,6):", R.add(5, 6))                  # 4
    print("neg(3):", R.neg(3))                       # 4
    print("mul(3,5):", R.mul(3, 5))                  # 1
    print("is_equal(10,3):", R.is_equal(10, 3))      # True

    # unified power (multiplicative)
    print("power(3,4):", R.power(3, 4))              # 4
    print("power([1,2,3,6], 3):", R.power([1, 2, 3, 6], 3))          # [1, 1, 6, 6]
    print("power([2,3,4,5],[0,1,2,3]):", R.power([2, 3, 4, 5], [0, 1, 2, 3]))  # [1, 3, 2, 6]