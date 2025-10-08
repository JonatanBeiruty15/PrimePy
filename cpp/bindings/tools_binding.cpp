#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <filesystem>
#include "../src/tools/number_theory/prime_table_io.h"

namespace py = pybind11;
using namespace primepy::tools::number_theory;

void bind_tools(py::module_& m_tools) {
    // resolve default path
    try {
        const std::string module_path = py::cast<std::string>(m_tools.attr("__file__"));
        std::filesystem::path p(module_path);
        auto data_path = p.parent_path().parent_path() / "data" / "primes_u32_le.bin";
        set_primes_file_path(data_path.string());
    } catch (...) {}

    m_tools.def("set_primes_file", &set_primes_file_path, py::arg("path"));
    m_tools.def("primes_table_size", &primes_table_size);
    m_tools.def("nth_prime", [](std::size_t n){ return nth_prime(n); }, py::arg("n"),
                py::call_guard<py::gil_scoped_release>());
    m_tools.def("first_primes", [](std::size_t k){ return first_primes(k); }, py::arg("k"),
                py::call_guard<py::gil_scoped_release>());
    m_tools.def("load_prime_number_batch",
                [](const std::vector<std::size_t>& idx){ return load_prime_number_batch(idx); },
                py::arg("indices"),
                py::call_guard<py::gil_scoped_release>());
}