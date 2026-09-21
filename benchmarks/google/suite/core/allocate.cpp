// Stack Core Benchmark Suite — Allocate
// Measures Stack allocate() performance against stdStack,
// the standard library's own linear/bump allocator.
//
// Each case owns a buffer sized generously above what its fixed
// iteration count can consume, so repeated calls never exhaust capacity
// mid-benchmark — only the steady-state bump-allocation path is
// measured, with no branching to the failure path. Iteration counts are
// pinned per case (rather than left to Google Benchmark's adaptive
// scaling) precisely to guarantee that bound.
//
// Covers:
// - allocate() of a small, word-sized block at the default alignment
// - allocate() of a larger, cache-line-sized block at the default alignment
// - allocate() of a small block at an over-aligned boundary (64 bytes)

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kSmallSize = sizeof(int);
constexpr std::size_t kLargeSize = 256;
constexpr std::size_t kOverAlignment = 64;

// Worst-case bytes consumed must stay under kCapacityBytes:
//   small:   1,000,000 x  <=16 B =  16 MB
//   large:     200,000 x   256 B =  51 MB
//   aligned:   500,000 x    64 B =  32 MB
constexpr benchmark::IterationCount kSmallIterations = 1'000'000;
constexpr benchmark::IterationCount kLargeIterations = 200'000;
constexpr benchmark::IterationCount kAlignedIterations = 500'000;
} // namespace

// Measures Stack allocate() of a small, word-sized block at the default
// alignment — the cheapest possible call through the hot path.
static void allocate_small_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kSmallSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_small_stack)->Iterations(kSmallIterations);

// Measures stdStack allocate() of a small, word-sized block at the
// default alignment.
static void allocate_small_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(kSmallSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_small_std)->Iterations(kSmallIterations);

// Measures Stack allocate() of a larger, cache-line-sized block at the
// default alignment.
static void allocate_large_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kLargeSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_large_stack)->Iterations(kLargeIterations);

// Measures stdStack allocate() of a larger, cache-line-sized block at
// the default alignment.
static void allocate_large_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(kLargeSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_large_std)->Iterations(kLargeIterations);

// Measures Stack allocate() of a small block at an over-aligned
// (64-byte) boundary — exercises the alignment padding path on every
// call.
static void allocate_aligned_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes, kOverAlignment);

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kSmallSize, kOverAlignment);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_aligned_stack)->Iterations(kAlignedIterations);

// Measures stdStack allocate() of a small block at an over-aligned
// (64-byte) boundary.
static void allocate_aligned_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(kSmallSize, kOverAlignment);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_aligned_std)->Iterations(kAlignedIterations);
