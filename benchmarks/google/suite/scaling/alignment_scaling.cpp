// Scaling Benchmark Suite — Alignment Scaling
// Measures Stack allocate() performance against stdStack as the
// requested alignment grows, holding allocation size fixed.
//
// Stack requires that any per-call alignment not exceed the alignment
// the allocator was constructed with, so each case constructs Stack
// with the same alignment it then requests on every call. As with
// capacity_growth.cpp, every repetition resets first so a single
// buffer can absorb an unbounded number of repetitions regardless of
// how much padding the largest alignment tier consumes per call.
//
// Covers:
// - allocate() at 4-byte alignment
// - allocate() at 64-byte alignment
// - allocate() at 4096-byte alignment

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
constexpr std::size_t kSmallAlignment = 4;
constexpr std::size_t kMediumAlignment = 64;
constexpr std::size_t kLargeAlignment = 4096;
} // namespace

// Measures Stack allocate() at 4-byte alignment.
static void alignment_4_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes, kSmallAlignment);

    for (auto _ : state) {
        cSrc.reset();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize, kSmallAlignment));
    }
}
BENCHMARK(alignment_4_stack);

// Measures stdStack allocate() at 4-byte alignment.
static void alignment_4_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        sSrc.release();
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize, kSmallAlignment));
    }
}
BENCHMARK(alignment_4_std);

// Measures Stack allocate() at 64-byte alignment.
static void alignment_64_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes, kMediumAlignment);

    for (auto _ : state) {
        cSrc.reset();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize, kMediumAlignment));
    }
}
BENCHMARK(alignment_64_stack);

// Measures stdStack allocate() at 64-byte alignment.
static void alignment_64_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        sSrc.release();
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize, kMediumAlignment));
    }
}
BENCHMARK(alignment_64_std);

// Measures Stack allocate() at 4096-byte alignment.
static void alignment_4096_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes, kLargeAlignment);

    for (auto _ : state) {
        cSrc.reset();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize, kLargeAlignment));
    }
}
BENCHMARK(alignment_4096_stack);

// Measures stdStack allocate() at 4096-byte alignment.
static void alignment_4096_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        sSrc.release();
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize, kLargeAlignment));
    }
}
BENCHMARK(alignment_4096_std);
