#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "primepy/algebra/algebraic_structures_exp.h"
#include "primepy/algebra/algebraic_structures_utils.h"

using namespace primepy::algebra;

namespace {

using Clock = std::chrono::steady_clock;

Integer raw_int(const Element& element) {
    return std::get<Integer>(element.data());
}

double milliseconds(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

Element naive_power_by_repeated_operation(const Group& group,
                                          const Element& base,
                                          long long exponent) {
    if (exponent < 0) {
        throw std::invalid_argument("Speed test only uses non-negative exponents.");
    }

    Element result = group.identity();
    for (long long i = 0; i < exponent; ++i) {
        result = group.operate(result, base);
    }
    return result;
}

void print_result(const std::string& label, double ms, const Integer& checksum) {
    std::cout << label << ": " << ms << " ms"
              << " | checksum = " << checksum.get_str() << '\n';
}

} // namespace

int main(int argc, char** argv) {
    const long long exponent = 3495;
    const int modulus = 1'000'003;

    const int scalar_repetitions = argc > 1 ? std::atoi(argv[1]) : 1'000;
    const int batch_size = argc > 2 ? std::atoi(argv[2]) : 20'000;

    auto group = std::make_shared<MultiplicativeModGroup>(modulus);
    const Element base = group->element(123456);

    std::cout << "Speed algebra test: multiplicative group (Z/" << modulus << "Z)^*\n";
    std::cout << "Exponent: " << exponent << '\n';
    std::cout << "Scalar repetitions: " << scalar_repetitions << '\n';
    std::cout << "Batch size: " << batch_size << "\n\n";

    // ------------------------------------------------------------------
    // Small integer arithmetic: native int64_t versus GMP mpz_class.
    // This gives a rough cost estimate for using reliable arbitrary-size
    // integers on values that would have fit in a machine word.
    // ------------------------------------------------------------------
    const int arithmetic_repetitions = batch_size * 200;

    std::int64_t native_value = 123456;
    std::int64_t native_checksum = 0;
    auto start = Clock::now();
    for (int i = 0; i < arithmetic_repetitions; ++i) {
        native_value = (native_value * 37 + i) % modulus;
        native_checksum += native_value;
    }
    auto end = Clock::now();
    const double native_arithmetic_ms = milliseconds(start, end);

    Integer gmp_value = 123456;
    Integer gmp_checksum = 0;
    start = Clock::now();
    for (int i = 0; i < arithmetic_repetitions; ++i) {
        gmp_value = (gmp_value * 37 + i) % modulus;
        gmp_checksum += gmp_value;
    }
    end = Clock::now();
    const double gmp_arithmetic_ms = milliseconds(start, end);
    const Integer native_checksum_integer(std::to_string(native_checksum));

    if (gmp_checksum != native_checksum_integer) {
        std::cerr << "Native/GMP arithmetic checksums differ: native=" << native_checksum
                  << ", GMP=" << gmp_checksum.get_str() << '\n';
        return 1;
    }

    print_result("Native int64 modular arithmetic",
                 native_arithmetic_ms,
                 native_checksum_integer);
    print_result("GMP Integer modular arithmetic", gmp_arithmetic_ms, gmp_checksum);
    if (native_arithmetic_ms > 0.0) {
        std::cout << "GMP slowdown on small integers: "
                  << gmp_arithmetic_ms / native_arithmetic_ms << "x\n\n";
    }

    // ------------------------------------------------------------------
    // Scalar power: repeated operation versus the group's power method.
    // In this group, repeated operation means a * a * ... * a modulo p.
    // The general group power method uses binary exponentiation instead.
    // ------------------------------------------------------------------
    Integer naive_scalar_checksum = 0;
    start = Clock::now();
    for (int i = 0; i < scalar_repetitions; ++i) {
        Element out = naive_power_by_repeated_operation(*group, base, exponent);
        naive_scalar_checksum += raw_int(out);
    }
    end = Clock::now();
    const double naive_scalar_ms = milliseconds(start, end);

    Integer power_scalar_checksum = 0;
    start = Clock::now();
    for (int i = 0; i < scalar_repetitions; ++i) {
        Element out = group->power(base, exponent);
        power_scalar_checksum += raw_int(out);
    }
    end = Clock::now();
    const double power_scalar_ms = milliseconds(start, end);

    if (naive_scalar_checksum != power_scalar_checksum) {
        std::cerr << "Scalar checksums differ: naive=" << naive_scalar_checksum.get_str()
                  << ", power=" << power_scalar_checksum.get_str() << '\n';
        return 1;
    }

    print_result("Scalar repeated operate", naive_scalar_ms, naive_scalar_checksum);
    print_result("Scalar group->power", power_scalar_ms, power_scalar_checksum);
    if (power_scalar_ms > 0.0) {
        std::cout << "Scalar speedup: " << naive_scalar_ms / power_scalar_ms << "x\n";
    }

    // ------------------------------------------------------------------
    // Batch power: a normal for-loop versus the vector batch API.
    // With OpenMP enabled, group->power(bases, exponent) parallelizes the
    // outer loop inside GroupUtils.
    // ------------------------------------------------------------------
    std::vector<Element> bases;
    bases.reserve(static_cast<std::size_t>(batch_size));
    for (int i = 0; i < batch_size; ++i) {
        bases.push_back(group->element((i % (modulus - 1)) + 1));
    }

    std::vector<Element> manual_loop_results;
    manual_loop_results.reserve(bases.size());

    start = Clock::now();
    for (const Element& item : bases) {
        manual_loop_results.push_back(group->power(item, exponent));
    }
    end = Clock::now();
    const double manual_loop_batch_ms = milliseconds(start, end);

    start = Clock::now();
    std::vector<Element> batch_results = group->power(bases, exponent);
    end = Clock::now();
    const double batch_method_ms = milliseconds(start, end);

    if (manual_loop_results.size() != batch_results.size()) {
        std::cerr << "Batch result sizes differ.\n";
        return 1;
    }

    Integer manual_loop_batch_checksum = 0;
    Integer batch_method_checksum = 0;
    for (std::size_t i = 0; i < manual_loop_results.size(); ++i) {
        const auto manual_loop_value = raw_int(manual_loop_results[i]);
        const auto batch_value = raw_int(batch_results[i]);
        if (manual_loop_value != batch_value) {
            std::cerr << "Batch mismatch at index " << i
                      << ": manual_loop=" << manual_loop_value.get_str()
                      << ", batch=" << batch_value.get_str() << '\n';
            return 1;
        }
        manual_loop_batch_checksum += manual_loop_value;
        batch_method_checksum += batch_value;
    }

    std::cout << '\n';
    print_result("Manual loop using group->power", manual_loop_batch_ms, manual_loop_batch_checksum);
    print_result("group->power(batch)", batch_method_ms, batch_method_checksum);
    if (batch_method_ms > 0.0) {
        std::cout << "Batch speedup: " << manual_loop_batch_ms / batch_method_ms << "x\n";
    }

    // ------------------------------------------------------------------
    // Batch power with one exponent per base.
    // This exercises group->power(bases, exponents), which parallelizes the
    // same outer loop but reads a different exponent for each element.
    // ------------------------------------------------------------------
    std::vector<long long> exponents;
    exponents.reserve(bases.size());
    for (int i = 0; i < batch_size; ++i) {
        exponents.push_back(1'000 + ((static_cast<long long>(i) * 37) % 5'000));
    }

    std::vector<Element> manual_elemwise_results;
    manual_elemwise_results.reserve(bases.size());

    start = Clock::now();
    for (std::size_t i = 0; i < bases.size(); ++i) {
        manual_elemwise_results.push_back(group->power(bases[i], exponents[i]));
    }
    end = Clock::now();
    const double manual_elemwise_batch_ms = milliseconds(start, end);

    start = Clock::now();
    std::vector<Element> elemwise_batch_results = group->power(bases, exponents);
    end = Clock::now();
    const double elemwise_batch_method_ms = milliseconds(start, end);

    if (manual_elemwise_results.size() != elemwise_batch_results.size()) {
        std::cerr << "Elementwise batch result sizes differ.\n";
        return 1;
    }

    Integer manual_elemwise_checksum = 0;
    Integer elemwise_batch_checksum = 0;
    for (std::size_t i = 0; i < manual_elemwise_results.size(); ++i) {
        const auto manual_value = raw_int(manual_elemwise_results[i]);
        const auto batch_value = raw_int(elemwise_batch_results[i]);
        if (manual_value != batch_value) {
            std::cerr << "Elementwise batch mismatch at index " << i
                      << ": manual=" << manual_value.get_str()
                      << ", batch=" << batch_value.get_str() << '\n';
            return 1;
        }
        manual_elemwise_checksum += manual_value;
        elemwise_batch_checksum += batch_value;
    }

    std::cout << '\n';
    print_result("Manual loop using group->power with different exponents",
                 manual_elemwise_batch_ms,
                 manual_elemwise_checksum);
    print_result("group->power(batch, exponents)",
                 elemwise_batch_method_ms,
                 elemwise_batch_checksum);
    if (elemwise_batch_method_ms > 0.0) {
        std::cout << "Different-exponent batch speedup: "
                  << manual_elemwise_batch_ms / elemwise_batch_method_ms << "x\n";
    }

#ifdef _OPENMP
    std::cout << "\nOpenMP: enabled\n";
#else
    std::cout << "\nOpenMP: disabled\n";
#endif

    return 0;
}
