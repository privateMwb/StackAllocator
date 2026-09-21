// Utility Benchmark Suite — Stats Toggle
// Measures the cost delta of allocate() with EnableStats enabled
// versus disabled.
//
// EnableStats is a compile-time template parameter, not a runtime
// flag: Stack<false>'s stats_ member is a zero-size Empty via
// [[no_unique_address]], and every stat-recording call site is
// compiled out entirely via `if constexpr`. There is no runtime
// equivalent to pair this against, so both cases run solo and are
// meant to be read side by side — the whole point of this suite is
// confirming the "off" case carries no residual cost at all.
//
// Neither case resets, so both iteration counts are pinned to the same
// value (rather than left to Google Benchmark's adaptive scaling) to
// guarantee the buffer can't run out mid-measurement and to keep the
// two cases directly comparable.
//
// Covers:
// - allocate() with EnableStats = false
// - allocate() with EnableStats = true

#include <support/framework.h>

#include <benchmark/benchmark.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 64 * 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);

// Worst case: 1,000,000 x <=16 B = 16 MB < kSize.
constexpr benchmark::IterationCount kIterations = 1'000'000;
} // namespace

// Measures allocate() with statistics disabled.
static void allocate_stats_off(benchmark::State& state) {
    Stack<false> cSrc(kSize);

    for (auto _ : state) {
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(allocate_stats_off)->Iterations(kIterations);

// Measures allocate() with statistics enabled.
static void allocate_stats_on(benchmark::State& state) {
    Stack<true> cSrc(kSize);

    for (auto _ : state) {
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(allocate_stats_on)->Iterations(kIterations);
