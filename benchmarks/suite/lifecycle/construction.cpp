// Lifecycle Benchmark Suite — Construction
// Measures Stack's constructor/destructor cost against stdStack's,
// the standard library's own linear/bump allocator.
//
// Both sides construct a fresh instance sized for N bytes and
// immediately let it go out of scope, so each repetition is a full,
// independent construct-then-destroy pair. Note this compares two
// different construction strategies, not just two implementations:
// Stack eagerly allocates its buffer at construction, while
// stdStack (std::pmr::monotonic_buffer_resource) can defer its first
// real allocation until the first call to allocate() — so a fixed-size
// stdStack constructed this way does no upfront heap work at all,
// which the numbers here should be read in light of.
//
// Covers:
// - constructing (and destroying) an empty allocator sized for N bytes

#include <support/framework.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kConstructBytes = 64 * 1024;
} // namespace

// Measures constructing an empty allocator sized for a fixed byte
// count, immediately followed by destruction at scope exit.
static void bench_construction() {
    auto c = [&] {
        Stack<false> s(kConstructBytes);
        doNotOptimize(&s);
    };

    auto s = [&] {
        stdStack r(kConstructBytes);
        doNotOptimize(&r);
    };

    BENCH("construction", c, s);
}

// Executes all construction benchmark cases.
static void run_benchmarks() {
    bench_construction();
}

REGISTER_BENCH_SUITE();
