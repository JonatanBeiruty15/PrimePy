#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace primepy::tools::number_theory {

// Configure absolute path to the primes table binary (uint32 little-endian)
void set_primes_file_path(std::string path);

// Table size (# of primes), derived from file length / 4. Throws on errors.
std::size_t primes_table_size();

// Read n-th prime (1-based) via single seek+read. Throws on range/IO errors.
std::uint32_t nth_prime(std::size_t n);

// Read first k primes in one sequential read (clamps k to table size).
std::vector<std::uint32_t> first_primes(std::size_t k);

// Read arbitrary 1-based indices in the given order. Coalesces only local runs.
std::vector<std::uint32_t>
load_prime_number_batch(const std::vector<std::size_t>& indices);

} // namespace primepy::tools::number_theory