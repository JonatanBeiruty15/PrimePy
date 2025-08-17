# PrimePy

**PrimePy** is a modern, modular number theory library designed with a clean Pythonic interface and a high-performance C++ backend. It provides foundational algebraic and analytic tools for experimentation, research, and education in number theory, cryptography, and related areas.

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
├── cpp/                       # C++ source code
│   ├── include/               # Header files (e.g., algebra, analytic, bindings)
│   ├── src/                   # C++ implementations
│   ├── bindings/              # Pybind11 wrappers
│   ├── tests/                 # C++ unit tests
│   └── CMakeLists.txt         # Build configuration
│
├── primepy/                   # Python interface (Pybind11-connected)
├── tests/                     # Python unit tests
│
├── docs/                      # Project documentation (LaTeX)
│   └── main.tex               # Master document
│
├── requirements.txt           # Python dependencies
├── pyproject.toml             # Optional Python packaging configuration
└── README.md                  # This file


