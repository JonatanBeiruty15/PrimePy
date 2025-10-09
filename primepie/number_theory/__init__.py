"""
primepie.number_theory
======================

Number theory subpackage: fast access to precomputed primes,
and other number-theoretic utilities.
"""


from .primes import (
    Primes,
    nth_prime,
    first_primes,
    primes_table_size,
    set_primes_table_path,
)

__all__ = [
    "Primes",
    "nth_prime",
    "first_primes",
    "primes_table_size",
    "set_primes_table_path",
]