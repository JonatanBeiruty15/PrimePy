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

    def power(self, base, exponent):
        return self._backend.power(base, exponent)

    def power_batch(self, bases, exponent):
        return self._backend.power(bases, exponent)

    def power_elementwise(self, bases, exponents):
        return self._backend.power(bases, exponents)
    


if __name__ == "__main__":
    G = AdditiveModGroup(7)
    print(G.identity())                  # 0
    print(G.inverse(3))                  # 4
    print(G.operate(3, 6))               # 2
    print(G.power(2, 3))                 # 1
    print(G.power_batch([1, 2, 3], 2))   # [2, 4, 6]
    print(G.power_elementwise([1, 2, 3], [0, 1, 2]))  # [0, 2, 2]