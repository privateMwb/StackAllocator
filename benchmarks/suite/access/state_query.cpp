// Access Benchmark Suite — State Query
// Measures Stack's introspection accessors.
//
// Each case builds its source stack once and allocates partway into
// it, outside the timed lambda, so used()/remaining() return a
// realistic non-zero value — only the repeated accessor call itself is
// measured. std::pmr::monotonic_buffer_resource exposes none of these
// queries, so every case runs solo.
//
// Covers:
// - used()
// - remaining()
// - capacity()

#include <support/framework.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures used() on a partially-filled stack.
static void bench_used() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    auto c = [&] {
        std::size_t v = cSrc.used();
        doNotOptimize(v);
    };

    BENCH_SOLO("used()", c);
}

// Measures remaining() on a partially-filled stack.
static void bench_remaining() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    auto c = [&] {
        std::size_t v = cSrc.remaining();
        doNotOptimize(v);
    };

    BENCH_SOLO("remaining()", c);
}

// Measures capacity() on a partially-filled stack.
static void bench_capacity() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    auto c = [&] {
        std::size_t v = cSrc.capacity();
        doNotOptimize(v);
    };

    BENCH_SOLO("capacity()", c);
}

// Executes all state query benchmark cases.
static void run_benchmarks() {
    bench_used();
    std::cout << "\n";

    bench_remaining();
    std::cout << "\n";

    bench_capacity();
}

REGISTER_BENCH_SUITE();
