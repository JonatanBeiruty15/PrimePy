"""
primepie.number_theory.primes
=============================

High-level Python API for accessing precomputed prime numbers efficiently.
Backed by the C++ core (_core.tools).
"""

from __future__ import annotations
from typing import Iterable, Iterator, List, overload, Sequence, Union, overload
from ..core import _core

# ---------------------------
# Low-level wrappers
# ---------------------------

def _table_size() -> int:
    return _core.tools.primes_table_size()

def _nth_prime_1based(n: int) -> int:
    if n < 1:
        raise ValueError("n must be >= 1 (1-based indexing)")
    return _core.tools.nth_prime(n)

def _first_primes(k: int) -> List[int]:
    if k < 0:
        raise ValueError("k must be >= 0")
    return _core.tools.first_primes(k)

def _batch(indices: Sequence[int]) -> List[int]:
    if not indices:
        return []
    if any(i < 1 for i in indices):
        raise ValueError("indices are 1-based; all must be >= 1")
    return _core.tools.load_prime_number_batch(list(indices))



# ------------------------------------------------
# Public API
# ------------------------------------------------


def primes_table_size() -> int:
    """Return the total number of precomputed primes in the binary table."""
    return _table_size()


@overload
def nth_prime(n: int) -> int: ...
@overload
def nth_prime(n: Sequence[int]) -> List[int]: ...


def nth_prime(n: Union[int, Sequence[int]]) -> Union[int, List[int]]:
    """
    Return the 1-based n-th prime (or a batch of primes).

    Parameters
    ----------
    n : int or Sequence[int]
        - If an integer `n >= 1`: returns the n-th prime.
        - If a sequence of integers (each >= 1): returns a list of primes
          corresponding to those indices, preserving input order.

    Returns
    -------
    int or List[int]
        The requested prime(s).

    Examples
    --------
    >>> nth_prime(10)
    29
    >>> nth_prime([1, 5, 10])
    [2, 11, 29]
    """
    if isinstance(n, int):
        if n < 1:
            raise ValueError("n must be >= 1 (1-based indexing)")
        return _nth_prime_1based(n)

    if isinstance(n, (list, tuple)) or hasattr(n, "__iter__"):
        if isinstance(n, (str, bytes, bytearray)):
            raise TypeError("nth_prime() does not accept strings; pass int or a sequence of ints.")
        idx = list(n)
        if any((not isinstance(i, int)) for i in idx):
            raise TypeError("nth_prime(sequence): all indices must be integers.")
        if any(i < 1 for i in idx):
            raise ValueError("nth_prime(sequence): indices are 1-based and must be >= 1.")
        return _batch(idx)

    raise TypeError("nth_prime() expects an int or a sequence of ints.")


def first_primes(k: int) -> List[int]:
    """
    Return the first `k` primes.

    Parameters
    ----------
    k : int
        Number of primes to retrieve (k >= 0).

    Returns
    -------
    List[int]
        The first `k` prime numbers.
    """
    if k < 0:
        raise ValueError("k must be >= 0")
    return _first_primes(k)


def set_primes_table_path(path: str) -> None:
    """
    Override the default path to the primes table binary file.

    Parameters
    ----------
    path : str
        Absolute or relative path to the binary file containing precomputed primes.
    """
    _core.tools.set_primes_file(path)

# ------------------------------------------------
# A friendly sliceable/iterable prime table view
# ------------------------------------------------
class _PrimeTableView:
    """A read-only, sliceable view over the precomputed primes table."""

    def __len__(self) -> int:
        return primes_table_size()

    # --- indexing ---
    @overload
    def __getitem__(self, idx: int) -> int: ...
    @overload
    def __getitem__(self, idx: slice) -> List[int]: ...
    @overload
    def __getitem__(self, idx: Sequence[int]) -> List[int]: ...

    def __getitem__(self, idx: Union[int, slice, Sequence[int]]) -> Union[int, List[int]]:
        if isinstance(idx, int):
            if idx < 1:
                raise IndexError("primes[i] uses 1-based indexing")
            return _nth_prime_1based(idx)

        if isinstance(idx, slice):
            start = 0 if idx.start is None else idx.start
            stop  = len(self) if idx.stop is None else idx.stop
            step  = 1 if idx.step is None else idx.step
            if step != 1:
                raise ValueError("only step=1 supported for slices")
            if start < 0 or stop < 0:
                raise IndexError("negative indices not supported")
            if stop <= start:
                return []
            out = _first_primes(stop)
            return out[start:]

        # Sequence[int] / list of indices
        if isinstance(idx, Iterable) and not isinstance(idx, (str, bytes, bytearray)):
            return _batch(list(idx))

        raise TypeError("Unsupported index type for primes")

    # --- callable sugar: primes(n) / primes([…]) ---
    @overload
    def __call__(self, n: int) -> int: ...
    @overload
    def __call__(self, n: Sequence[int]) -> List[int]: ...
    def __call__(self, n: Union[int, Sequence[int]]) -> Union[int, List[int]]:
        return nth_prime(n)

    # --- simple iterator (chunked reads) ---
    def __iter__(self) -> Iterator[int]:
        total = len(self)
        chunk = 1000
        read = 0
        while read < total:
            part = min(chunk, total - read)
            batch = _first_primes(read + part)
            for v in batch[read:]:
                yield v
            read += part

# Singleton instance
Primes = _PrimeTableView()

__all__ = [
    "primes",
    "nth_prime",
    "first_primes",
    "primes_batch",
    "primes_table_size",
    "set_primes_table_path",
]



if __name__ == '__main__':
   
    print(_core.__file__)
    print(hasattr(_core, "tools"))       # should be True
    print(dir(_core.tools))  