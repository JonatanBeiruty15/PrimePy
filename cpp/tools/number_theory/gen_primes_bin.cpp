// ============================================================================
// PrimePy tool: generate a binary table of the first N primes
// Usage: gen_primes_bin <N> <out_path>
// Example: ./gen_primes_bin 100000 primes_u32_le.bin
// Writes N primes as uint32_t in little-endian order.
// ============================================================================
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace std;

// ----------------------------
// Simple sieve + fallback
// ----------------------------
static vector<uint32_t> first_n_primes(size_t N) {
    if (N == 0) return {};
    if (N <= 6) {
        static const uint32_t s[] = {2,3,5,7,11,13};
        return {s, s + N};
    }

    double n = static_cast<double>(N);
    // upper bound for the N-th prime (approx from PNT)
    size_t limit = static_cast<size_t>(max(20.0, n * (log(n) + log(log(n))) + 32.0));

    vector<bool> composite(limit + 1, false);
    vector<uint32_t> primes;
    primes.reserve(N);

    for (size_t i = 2; i <= limit && primes.size() < N; ++i) {
        if (!composite[i]) {
            primes.push_back(static_cast<uint32_t>(i));
            if (i * i <= limit)
                for (size_t j = i * i; j <= limit; j += i)
                    composite[j] = true;
        }
    }

    // fallback if upper bound too low (rare)
    for (uint32_t x = static_cast<uint32_t>(limit + 1); primes.size() < N; ++x) {
        bool ok = true;
        for (uint32_t p : primes) {
            if ((uint64_t)p * p > x) break;
            if (x % p == 0) { ok = false; break; }
        }
        if (ok) primes.push_back(x);
    }

    return primes;
}

// ----------------------------
// main()
// ----------------------------
int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N> <out.bin>\n", argv[0]);
        return 1;
    }

    size_t N = stoull(argv[1]);
    const char* out = argv[2];

    auto primes = first_n_primes(N);

    FILE* f = fopen(out, "wb");
    if (!f) { perror("fopen"); return 1; }

    // write as uint32_t little-endian
    for (uint32_t p : primes) {
        uint32_t le = p;
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        le = __builtin_bswap32(le);
    #endif
        fwrite(&le, sizeof(uint32_t), 1, f);
    }
    fclose(f);

    fprintf(stderr, "✅ Wrote %zu primes to %s. Last prime = %u\n",
            primes.size(), out, primes.back());
    return 0;
}