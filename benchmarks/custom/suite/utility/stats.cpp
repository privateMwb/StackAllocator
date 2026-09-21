// Utility Benchmark Suite — Stats
// Measures Stack<true>::getStats() performance.
//
// Each case builds its source stack once and allocates into it,
// outside the timed lambda — only the repeated accessor call itself
// is measured. std::pmr::monotonic_buffer_resource tracks no
// allocation statistics of its own, so this runs solo.
//
// Covers:
// - getStats() on a stack with prior allocations outstanding

#include <support/framework.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures getStats() on a stack with outstanding allocations.
static void bench_get_stats() {
    Stack<true> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    auto c = [&] {
        const auto& s = cSrc.getStats();
        doNotOptimize(s);
    };

    BENCH_SOLO("getStats()", c);
}

// Executes all stats benchmark cases.
static void run_benchmarks() {
    bench_get_stats();
}

REGISTER_BENCH_SUITE();
