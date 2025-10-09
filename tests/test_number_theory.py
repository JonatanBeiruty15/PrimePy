# tests/test_number_theory.py

import sys
import os
import pytest

# Ensure project root (folder containing 'primepie') is on sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from primepie.number_theory.primes import (
    primes_table_size,
    Primes,          # unified: accepts int or sequence[int]
    first_primes,
)

# Known small primes for reference
FIRST_10 = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29]


def test_table_size_basic():
    """Table size should be positive and reasonably large (we expect >= 10k)."""
    sz = primes_table_size()
    assert isinstance(sz, int) and sz > 0
    assert sz >= 10_000     # your CMake currently generates 100_000


def test_first_primes_10():
    """first_primes(10) must match the canonical first 10 primes."""
    assert first_primes(10) == FIRST_10


def test_nth_prime_small_positions():
    """Check several specific positions against well-known values."""
    assert Primes(1) == 2
    assert Primes(2) == 3
    assert Primes(6) == 13
    assert Primes(10) == 29
    assert Primes(100) == 541
    assert Primes(1_000) == 7919


def test_nth_prime_batch_non_consecutive_order_preserved():
    """Batch retrieval should preserve the given order, non-consecutive indices."""
    idx = [2, 4, 3, 10, 6]
    out = Primes(idx)
    assert out == [3, 7, 5, 29, 13]


def test_nth_prime_batch_consecutive_coalesced():
    """
    Consecutive runs (e.g., [100,101,102,103]) are read efficiently,
    but we only check for correctness here.
    """
    idx = [100, 101, 102, 103]
    out = Primes(idx)
    # compute by comparing with first_primes up to max index
    fp = first_primes(max(idx))
    assert out == [fp[i-1] for i in idx]


def test_first_primes_clamps_to_table():
    """
    Asking for more than table size should clamp silently in the binding
    (first_primes), returning the full table.
    """
    sz = primes_table_size()
    out = first_primes(sz + 12345)
    assert len(out) == sz
    # spot-check the last few are stable against direct nth calls
    if sz >= 5:
        tail_idx = [sz-4, sz-3, sz-2, sz-1, sz]
        assert [Primes(i) for i in tail_idx] == out[-5:]


def test_nth_prime_raises_out_of_range():
    """Requesting n > table_size should raise an exception."""
    sz = primes_table_size()
    with pytest.raises(Exception):
        _ = Primes(sz + 1)


def test_nth_prime_invalid_inputs():
    """Type errors / invalid values."""
    with pytest.raises(TypeError):
        _ = Primes("100")     # type: ignore
    with pytest.raises(ValueError):
        _ = Primes(0)         # 1-based indexing
    with pytest.raises(ValueError):
        _ = Primes([-1, 1, 2])  # negative index in batch