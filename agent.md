# PrimePie Agent Memory

Last updated: 2026-06-02

This file is a reminder for future work on PrimePie. It records what we changed
recently, what was verified, and what should probably happen next.

## Recent Work

- Enabled and verified an OpenMP build path using GCC on macOS.
  - Working build directory used locally: `cpp/build-openmp`
  - This build directory is generated and should not be committed.
- Added GMP-backed integer support in the algebra layer.
  - Integer-valued algebra elements now use GMP `mpz_class`.
  - Python integers are converted through the bindings.
- Reworked algebra around runtime `Element` objects.
  - Operations generally accept raw Python integers when unambiguous.
  - Operations return `Element` objects.
  - Use `print(x)` for algebraic display, and `x.value` for the raw Python value.
- Added/expanded algebra structures:
  - `AdditiveModGroup`
  - `MultiplicativeModGroup`
  - `DirectSumGroup`
  - `Integers`
  - `IntegersModRing`
  - `PolynomialRing`
- Added speed tests under `cpp/tests/speed_tests/Speed_algebra_test.cpp`.
  - Tests compare repeated operation vs fast power, batch power, and GMP vs native integer arithmetic.
- Split documentation into two documents:
  - `docs/mathematical_algorithmic_documentation`
  - `docs/practical_user_guide`
- Changed both LaTeX documents to `report`.
- Rewrote the practical guide Algebra chapter around two sections:
  - Groups
  - Rings
- Added a short GMP note in the practical guide Algebra chapter.
- Moved practical guide Installation to Appendix A with a TODO placeholder.
- Added `roadmap.txt` with the Stage 0 plan for Algebraic Number Theory and Analytic Number Theory.
- Committed and pushed the large recent work:
  - Commit: `9b0752b Update algebra core docs and roadmap`
  - Branch: `beta`

## Verified Commands

The following worked during recent development:

```bash
cmake -S cpp -B cpp/build-openmp \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-14 \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build cpp/build-openmp --target Speed_algebra_test tests_algebra _core -j 4

ctest --test-dir cpp/build-openmp --output-on-failure

PrimePyVenv/bin/python -m pytest
```

Recent test results:

- C++ tests passed through CTest.
- Python tests passed.
- Practical guide and mathematical/algorithmic docs compiled with `latexmk`.

## Current Uncommitted Changes

At the time this file was created, there were local changes after the last push:

- `.gitignore`
  - Added `cpp/build-*/` so `cpp/build-openmp/` is ignored.
- `notes.txt`
  - Removed the solved TODO about algebraic bindings and `mpower`.
  - Added portability TODOs.
- `agent.md`
  - This memory file.

These should be committed later when convenient.

## Important Next Steps

The most urgent next step is portability and packaging. The library currently
works locally, but a fresh clone on another machine may fail.

Main portability TODOs:

1. Fresh clone dependency problem
   - CMake expects `cpp/external/pybind11` and `cpp/external/eigen`.
   - These folders are ignored and not tracked by Git.
   - Choose one solution: Git submodules, CMake `FetchContent`, or system packages.

2. Python packaging problem
   - `pip install .` currently does not build the C++ `_core` extension.
   - `setup.py` only calls plain `setup()`.
   - Need a real CMake-backed Python build flow.

3. GMP portability
   - GMP works locally, but Windows support needs a clear strategy.
   - Possible routes: MSYS2, vcpkg, or later a Boost.Multiprecision fallback.

4. OpenMP should be optional
   - Current CMake requires OpenMP when `PRIMEPY_ENABLE_OPENMP` is ON.
   - Better behavior: if OpenMP is unavailable, compile single-threaded.

5. Windows/MSVC endian issue
   - `prime_table_io.cpp` uses GCC/Clang endian macros and `__builtin_bswap32`.
   - This may fail on MSVC.
   - Replace with portable endian handling.

6. CMake install paths
   - Current CMake installs generated files into the source tree.
   - This is convenient locally but not correct for packaging.

## Stage 0 Roadmap Direction

The short-term mathematical goal is not full Galois theory yet.

Algebraic Number Theory Stage 0:

- Fields:
  - rational field `Q`
  - prime finite fields `F_p`
- Polynomial rings over fields:
  - polynomial division
  - polynomial gcd
  - derivative
  - squarefree tools
  - factorization over finite fields
- Modular polynomial reduction:
  - reduce `Q[x]` or `Z[x]` polynomials modulo good primes
  - factor modulo `p`
  - read factorization degrees as cycle types
- Basic permutations:
  - permutations in `S_n`
  - cycle decomposition
  - cycle type
  - sign
  - small generated groups

Analytic Number Theory Stage 0:

- Arithmetic functions:
  - `phi`
  - `mu`
  - divisor functions
  - von Mangoldt
  - Liouville
- Factorization support for moderate integers.
- Dirichlet convolution and finite Mobius inversion.
- Basic summatory functions.
- First zeta and Dirichlet L-function experiments using partial sums and Euler products.

## Mental Model To Remember

PrimePie should become:

- Exact algebraic computation for algebra and early algebraic number theory.
- Efficient range/batch computation for analytic number theory experiments.
- Reliable across machines, not just on the current macOS setup.

Before adding large new mathematics, make the build/install story honest.
