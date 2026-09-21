// Access Benchmark Suite — Ownership
// Measures Stack owns() performance.
//
// Each case builds its source stack once, outside the timed loop —
// only repeated ownership checks against an already-populated stack
// are measured. std::pmr::monotonic_buffer_resource exposes no
// ownership query, so both cases run solo.
//
// Covers:
// - owns() on a pointer that belongs to the stack (hit path)
// - owns() on a pointer that does not belong to the stack (miss path)

#include <support/framework.h>

#include <benchmark/benchmark.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures owns() on a pointer allocated from the stack.
static void owns_hit(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    std::byte* ptr = cSrc.allocate(sizeof(int));

    for (auto _ : state) {
        bool v = cSrc.owns(ptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(owns_hit);

// Measures owns() on a pointer that does not belong to the stack.
static void owns_miss(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(sizeof(int));

    int outside = 0;
    const void* ptr = &outside;

    for (auto _ : state) {
        bool v = cSrc.owns(ptr);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(owns_miss);
