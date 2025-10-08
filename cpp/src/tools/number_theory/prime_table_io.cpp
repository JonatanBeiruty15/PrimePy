// cpp/src/tools/number_theory/prime_table_io.cpp
#include "prime_table_io.h"  // header is in the same folder

#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace primepy::tools::number_theory {

namespace {
    std::string g_path; // absolute path to primes_u32_le.bin

    std::uint64_t file_size_bytes() {
        if (g_path.empty()) throw std::runtime_error("primes file path not set");
        std::ifstream in(g_path, std::ios::binary);
        if (!in) throw std::runtime_error("failed to open primes file: " + g_path);
        in.seekg(0, std::ios::end);
        auto sz = in.tellg();
        if (sz < 0) throw std::runtime_error("failed to stat primes file size");
        return static_cast<std::uint64_t>(sz);
    }
} // anonymous

void set_primes_file_path(std::string path) { g_path = std::move(path); }

std::size_t primes_table_size() {
    auto bytes = file_size_bytes();
    if (bytes % sizeof(std::uint32_t) != 0)
        throw std::runtime_error("invalid primes file size (not multiple of 4)");
    return static_cast<std::size_t>(bytes / sizeof(std::uint32_t));
}

std::uint32_t nth_prime(std::size_t n) {
    if (n == 0) throw std::invalid_argument("nth_prime: n is 1-based (n>=1)");
    if (g_path.empty()) throw std::runtime_error("primes file path not set");

    std::ifstream in(g_path, std::ios::binary);
    if (!in) throw std::runtime_error("failed to open primes file: " + g_path);

    const std::uint64_t offset = static_cast<std::uint64_t>(n - 1) * sizeof(std::uint32_t);

    in.seekg(0, std::ios::end);
    const auto sz = static_cast<std::uint64_t>(in.tellg());
    if (offset + sizeof(std::uint32_t) > sz)
        throw std::out_of_range("nth_prime: n exceeds precomputed table");

    in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

    std::uint32_t p = 0;
    in.read(reinterpret_cast<char*>(&p), sizeof(p));
    if (!in) throw std::runtime_error("failed to read prime at requested index");

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    p = __builtin_bswap32(p);
#endif
    return p;
}

std::vector<std::uint32_t> first_primes(std::size_t k) {
    if (g_path.empty()) throw std::runtime_error("primes file path not set");

    std::ifstream in(g_path, std::ios::binary);
    if (!in) throw std::runtime_error("failed to open primes file: " + g_path);

    in.seekg(0, std::ios::end);
    const auto sz = static_cast<std::uint64_t>(in.tellg());
    if (sz % sizeof(std::uint32_t) != 0)
        throw std::runtime_error("invalid primes file size (not multiple of 4)");

    const auto total = static_cast<std::size_t>(sz / sizeof(std::uint32_t));
    if (k > total) k = total;

    std::vector<std::uint32_t> out(k);
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(out.data()),
            static_cast<std::streamsize>(k * sizeof(std::uint32_t)));
    if (!in) throw std::runtime_error("failed to read first k primes");

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    for (auto &x : out) x = __builtin_bswap32(x);
#endif
    return out;
}

std::vector<std::uint32_t>
load_prime_number_batch(const std::vector<std::size_t>& indices) {
    if (g_path.empty()) throw std::runtime_error("primes file path not set");

    std::vector<std::uint32_t> out;
    out.reserve(indices.size());

    std::ifstream in(g_path, std::ios::binary);
    if (!in) throw std::runtime_error("failed to open primes file: " + g_path);

    in.seekg(0, std::ios::end);
    const auto sz = static_cast<std::uint64_t>(in.tellg());
    if (sz % sizeof(std::uint32_t) != 0)
        throw std::runtime_error("invalid primes file size (not multiple of 4)");
    const std::size_t total = static_cast<std::size_t>(sz / sizeof(std::uint32_t));

    for (std::size_t i = 0; i < indices.size(); ) {
        const std::size_t start_idx = indices[i];
        if (start_idx == 0) throw std::invalid_argument("indices are 1-based; found 0");
        if (start_idx > total) throw std::out_of_range("index exceeds precomputed prime table");

        // local run: start_idx, start_idx+1, ...
        std::size_t run_len = 1;
        while (i + run_len < indices.size()
               && indices[i + run_len] == start_idx + run_len) {
            ++run_len;
        }

        const std::uint64_t offset =
            static_cast<std::uint64_t>(start_idx - 1) * sizeof(std::uint32_t);
        in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

        if (run_len == 1) {
            std::uint32_t p = 0;
            in.read(reinterpret_cast<char*>(&p), sizeof(p));
            if (!in) throw std::runtime_error("failed to read prime (single) from file");
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            p = __builtin_bswap32(p);
#endif
            out.push_back(p);
            ++i;
        } else {
            std::vector<std::uint32_t> buf(run_len);
            in.read(reinterpret_cast<char*>(buf.data()),
                    static_cast<std::streamsize>(run_len * sizeof(std::uint32_t)));
            if (!in) throw std::runtime_error("failed to read primes (bulk) from file");
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            for (auto &x : buf) x = __builtin_bswap32(x);
#endif
            out.insert(out.end(), buf.begin(), buf.end());
            i += run_len;
        }
    }

    return out;
}

} // namespace primepy::tools::number_theory