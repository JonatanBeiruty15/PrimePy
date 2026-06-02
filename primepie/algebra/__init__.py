"""
primepie.algebra
================

Wrappers for algebraic structures backed by C++:
  - Group, AdditiveModGroup
  - Ring, Integers, IntegersModRing, PolynomialRing
  
"""

from .algebraic_structures import (
    Element,
    GroupProperty,
    Group,
    AdditiveModGroup,
    MultiplicativeModGroup,
    DirectSumGroup,
    Ring,
    Integers,
    IntegersModRing,
    PolynomialRing,
)

__all__ = [
    "Element",
    "GroupProperty",
    "Group",
    "AdditiveModGroup",
    "MultiplicativeModGroup",
    "DirectSumGroup",
    "Ring",
    "Integers",
    "IntegersModRing",
    "PolynomialRing",
]
