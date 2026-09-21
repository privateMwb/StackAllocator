// Scaling Benchmark Suite — Capacity Growth
// Measures Stack allocate() performance against stdStack as the
// underlying buffer capacity grows, holding allocation size fixed.
//
// Because the smallest tier here (4 KiB) can't safely absorb an
// unbounded number of repetitions the way a single generous fixed
// buffer can, every repetition resets first — bounding each rep's
// footprint to one allocation regardless of total call count. reset()
// / release() are both O(1) and roughly constant across all three
// tiers, so the relative comparison between tiers stays meaningful,
// though this suite is measuring "reset + allocate" rather than a pure
// allocate()-only cost. allocate() is itself O(1) regardless of buffer
// size, so the interesting question is whether that cost stays flat as
// capacity grows, or degrades from effects like cache/TLB pressure on
// a much larger backing buffer.
//
// Covers:
// - allocate() against a 4 KiB buffer
// - allocate() against a 1 MiB buffer
// - allocate() against a 64 MiB buffer

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kSmallCapacity = 4 * 1024;
constexpr std::size_t kMediumCapacity = 1024 * 1024;
constexpr std::size_t kLargeCapacity = 64 * 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures Stack allocate() against a 4 KiB buffer.
static void capacity_4kib_stack(benchmark::State& state) {
    Stack<false> cSrc(kSmallCapacity);

    for (auto _ : state) {
        cSrc.reset();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(capacity_4kib_stack);

// Measures stdStack allocate() against a 4 KiB buffer.
static void capacity_4kib_std(benchmark::State& state) {
    stdStack sSrc(kSmallCapacity);

    for (auto _ : state) {
        sSrc.release();
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
    }
}
BENCHMARK(capacity_4kib_std);

// Measures Stack allocate() against a 1 MiB buffer.
static void capacity_1mib_stack(benchmark::State& state) {
    Stack<false> cSrc(kMediumCapacity);

    for (auto _ : state) {
        cSrc.reset();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(capacity_1mib_stack);

// Measures stdStack allocate() against a 1 MiB buffer.
static void capacity_1mib_std(benchmark::State& state) {
    stdStack sSrc(kMediumCapacity);

    for (auto _ : state) {
        sSrc.release();
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
    }
}
BENCHMARK(capacity_1mib_std);

// Measures Stack allocate() against a 64 MiB buffer.
static void capacity_64mib_stack(benchmark::State& state) {
    Stack<false> cSrc(kLargeCapacity);

    for (auto _ : state) {
        cSrc.reset();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(capacity_64mib_stack);

// Measures stdStack allocate() against a 64 MiB buffer.
static void capacity_64mib_std(benchmark::State& state) {
    stdStack sSrc(kLargeCapacity);

    for (auto _ : state) {
        sSrc.release();
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
    }
}
BENCHMARK(capacity_64mib_std);
