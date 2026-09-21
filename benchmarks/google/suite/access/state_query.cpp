// Access Benchmark Suite — State Query
// Measures Stack's introspection accessors.
//
// Each case builds its source stack once and allocates partway into
// it, outside the timed loop, so used()/remaining() return a
// realistic non-zero value — only the repeated accessor call itself is
// measured. std::pmr::monotonic_buffer_resource exposes none of these
// queries, so every case runs solo.
//
// Covers:
// - used()
// - remaining()
// - capacity()

#include <support/framework.h>

#include <benchmark/benchmark.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures used() on a partially-filled stack.
static void used(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        std::size_t v = cSrc.used();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(used);

// Measures remaining() on a partially-filled stack.
static void remaining(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        std::size_t v = cSrc.remaining();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(remaining);

// Measures capacity() on a partially-filled stack.
static void capacity(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        std::size_t v = cSrc.capacity();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(capacity);
