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
// Covers:
// - allocate() with EnableStats = false
// - allocate() with EnableStats = true

#include <support/framework.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 64 * 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures allocate() with statistics disabled.
static void bench_allocate_stats_off() {
    Stack<false> cSrc(kSize);

    auto c = [&] { doNotOptimize(cSrc.allocate(kAllocSize)); };

    BENCH_SOLO("alloc stats off", c);
}

// Measures allocate() with statistics enabled.
static void bench_allocate_stats_on() {
    Stack<true> cSrc(kSize);

    auto c = [&] { doNotOptimize(cSrc.allocate(kAllocSize)); };

    BENCH_SOLO("alloc stats on", c);
}

// Executes all stats toggle benchmark cases.
static void run_benchmarks() {
    bench_allocate_stats_off();
    bench_allocate_stats_on();
}

REGISTER_BENCH_SUITE();
