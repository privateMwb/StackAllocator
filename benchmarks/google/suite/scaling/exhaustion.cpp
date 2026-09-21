// Scaling Benchmark Suite — Exhaustion
// Measures Stack allocate() performance with room to spare against
// the failure path once the allocator is completely full, paired
// against a bounded stdStack whose upstream is
// std::pmr::null_memory_resource().
//
// Stack's allocate() is noexcept: once full, every call is a bounds
// check that returns nullptr, with no state change — safe to repeat
// indefinitely. stdStack has no equivalent non-throwing failure path:
// once its fixed buffer is exhausted, it must ask its upstream for
// more, and an upstream of null_memory_resource() always throws
// std::bad_alloc. That asymmetry — a checked nullptr return versus a
// mandatory thrown exception — is the point of this suite, not just a
// side effect of the setup.
//
// The headroom cases never reset, so their iteration count is pinned
// (rather than left to Google Benchmark's adaptive scaling) to
// guarantee the buffer can't run out mid-measurement.
//
// Covers:
// - allocate() with room to spare
// - allocate() once the allocator is completely full

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>
#include <new>

using namespace StackPro;

namespace {
constexpr std::size_t kHeadroomCapacity = 64 * 1024 * 1024;

// Worst case: 1,000,000 x <=16 B = 16 MB < kHeadroomCapacity.
constexpr benchmark::IterationCount kHeadroomIterations = 1'000'000;

// Must be large enough to satisfy one allocation at the default
// alignment (alignof(std::max_align_t)) with zero bytes left over —
// otherwise the very first (untimed) setup allocation falls through
// to the upstream null_memory_resource() and throws before the timed
// loop ever starts.
constexpr std::size_t kFullCapacity = alignof(std::max_align_t);
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures Stack allocate() with plenty of remaining capacity — the
// steady-state success path.
static void exhaustion_headroom_stack(benchmark::State& state) {
    Stack<false> cSrc(kHeadroomCapacity);

    for (auto _ : state) {
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(exhaustion_headroom_stack)->Iterations(kHeadroomIterations);

// Measures stdStack allocate() with plenty of remaining capacity — the
// steady-state success path.
static void exhaustion_headroom_std(benchmark::State& state) {
    stdStack sSrc(kHeadroomCapacity);

    for (auto _ : state) {
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
    }
}
BENCHMARK(exhaustion_headroom_std)->Iterations(kHeadroomIterations);

// Measures Stack allocate() once the allocator is completely full.
// Stack returns nullptr with no state change, so the call is safe to
// repeat indefinitely.
static void exhaustion_full_stack(benchmark::State& state) {
    Stack<false> cSrc(kFullCapacity);
    benchmark::DoNotOptimize(cSrc.allocate(kFullCapacity));

    for (auto _ : state) {
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
    }
}
BENCHMARK(exhaustion_full_stack);

// Measures stdStack allocate() once the allocator is completely full.
// stdStack, bounded with a null_memory_resource() upstream, must throw
// std::bad_alloc on every call.
static void exhaustion_full_std(benchmark::State& state) {
    alignas(std::max_align_t) std::byte buffer[kFullCapacity];
    stdStack sSrc(buffer, sizeof(buffer), std::pmr::null_memory_resource());

    // This must succeed exactly, consuming the entire buffer with
    // nothing left over. If it throws here, the buffer above is sized
    // or aligned incorrectly for this standard library implementation.
    try {
        benchmark::DoNotOptimize(sSrc.allocate(kFullCapacity));
    } catch (const std::bad_alloc&) {
        AP_ASSERT(false && "setup allocation should never fail here");
        throw;
    }

    for (auto _ : state) {
        try {
            benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
        } catch (const std::bad_alloc&) {
            // Expected: upstream is null_memory_resource().
        }
    }
}
BENCHMARK(exhaustion_full_std);
