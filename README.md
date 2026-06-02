# PrimePie 🥧

**PrimePie** is a modern, modular number theory library designed with a clean Pythonic interface and a high-performance C++ backend. It provides foundational algebraic and analytic tools for experimentation, research, and education in number theory, cryptography, and related areas.

---

## 📌 Project Principles

- **🐍 Pythonic Interface**  
  A simple and flexible API that feels natural in Python.

- **⚙️ High-Performance C++ Core**  
  Core computations are implemented in C++ for maximum performance.

- **📊 Visualization-Ready**  
  Designed to integrate with plotting libraries for visual experimentation.

- **🧩 Minimal & General Abstractions**  
  Mathematical structures (like groups, rings, fields) are implemented in a clean, extensible, and general way.

---
## 📁 Project Structure

```text
PrimePy/
├── cpp/                       # C++ sources and build system
│   ├── include/               # Public headers (algebra, number theory, …)
│   ├── src/                   # C++ implementations
│   ├── bindings/              # pybind11 wrappers (C++ ↔ Python)
│   ├── tests/                 # C++ unit tests
│   ├── external/              # Third-party deps (e.g., pybind11, Eigen)
│   ├── tools/                 # Dev/build helper scripts
│   └── CMakeLists.txt         # CMake build config
│
├── primepie/                  # Python package (front-end API)
│   ├── algebra/               # Pythonic algebra wrappers
│   ├── core/                  # Low-level Python shims to the C++ core
│   ├── data/                  # Static data / tables (if any)
│   └── number_theory/         # Number theory APIs (primes, nth_prime, …)
│
├── tests/                     # Python unit tests
│   ├── test_algebra.py
│   └── test_number_theory.py
│
├── docs/                      # Documentation (LaTeX + assets)
│   ├── documentation/         # Theory/reference docs (PDFs live here)
│   ├── practical_user_guide/  # Practical guide (PDF here)
│   └── figures/               # Images used by the docs
│
├── requirements.txt           # Runtime Python dependencies
├── requirements-dev.txt       # Development/testing/build dependencies
├── pyproject.toml             # Python packaging config (build metadata)
├── setup.py                   # Minimal setuptools entry point
├── pytest.ini                 # Pytest configuration
├── LICENSE                    # License file
└── README.md                  # You are here
```
