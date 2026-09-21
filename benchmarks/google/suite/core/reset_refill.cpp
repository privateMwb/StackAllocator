// Stack Core Benchmark Suite — Reset & Refill
// Measures Stack reset() performance against stdStack's release(),
// the standard library's own bulk-reclaim operation.
//
// reset() is idempotent — it always sets the offset back to zero
// regardless of prior state — so it can be called repeatedly without
// any per-repetition setup. release() is the same for stdStack: it is
// always safe to call again even with nothing outstanding. The refill
// case additionally performs a small, fixed number of allocations
// after each reset, so every repetition starts from an empty allocator
// and walks back up to the same fixed high-water mark, bounded well
// under the buffer's capacity regardless of iteration count.
//
// Covers:
// - reset() alone, on an allocator with prior allocations outstanding
// - reset() followed by refilling to a fixed number of entries

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kRefillEntrySize = sizeof(int);
constexpr std::size_t kRefillEntryCount = 64;
} // namespace

// Measures Stack reset() alone, called repeatedly on an allocator that
// already has allocations outstanding from its initial fill.
static void reset_alone_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
        benchmark::DoNotOptimize(cSrc.allocate(kRefillEntrySize));
    }

    for (auto _ : state) {
        cSrc.reset();
    }
}
BENCHMARK(reset_alone_stack);

// Measures stdStack release() alone, called repeatedly on an allocator
// that already has allocations outstanding from its initial fill.
static void reset_alone_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
        benchmark::DoNotOptimize(sSrc.allocate(kRefillEntrySize));
    }

    for (auto _ : state) {
        sSrc.release();
    }
}
BENCHMARK(reset_alone_std);

// Measures Stack reset() followed by refilling to a fixed entry count —
// every repetition starts empty and walks back up to the same
// high-water mark.
static void reset_refill_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        cSrc.reset();
        for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
            benchmark::DoNotOptimize(cSrc.allocate(kRefillEntrySize));
        }
    }
}
BENCHMARK(reset_refill_stack);

// Measures stdStack release() followed by refilling to a fixed entry
// count — every repetition starts empty and walks back up to the same
// high-water mark.
static void reset_refill_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        sSrc.release();
        for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
            benchmark::DoNotOptimize(sSrc.allocate(kRefillEntrySize));
        }
    }
}
BENCHMARK(reset_refill_std);
