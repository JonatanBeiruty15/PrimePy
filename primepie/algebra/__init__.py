"""
primepie.algebra
================

 Wrappers for algebraic structures backed by C++:
  - Group, AdditiveModGroup
  - Ring, Integers, IntegersModRing
"""

from .algebraic_structures import (
    Group,
    AdditiveModGroup,
    Ring,
    Integers,
    IntegersModRing,
)

__all__ = [
    "Group",
    "AdditiveModGroup",
    "Ring",
    "Integers",
    "IntegersModRing",
]