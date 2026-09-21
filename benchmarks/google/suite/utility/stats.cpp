// Utility Benchmark Suite — Stats
// Measures Stack<true>::getStats() performance.
//
// Each case builds its source stack once and allocates into it,
// outside the timed loop — only the repeated accessor call itself
// is measured. std::pmr::monotonic_buffer_resource tracks no
// allocation statistics of its own, so this runs solo.
//
// Covers:
// - getStats() on a stack with prior allocations outstanding

#include <support/framework.h>

#include <benchmark/benchmark.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures getStats() on a stack with outstanding allocations.
static void stats_get(benchmark::State& state) {
    Stack<true> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        const auto& s = cSrc.getStats();
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(stats_get);
