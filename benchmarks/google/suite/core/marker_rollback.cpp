// Core Benchmark Suite — Marker & Rollback
// Measures Stack's getMarker() and freeToMarker() performance.
//
// Neither operation mutates capacity: getMarker() only reads the
// current offset, and freeToMarker() only overwrites it — repeating
// either indefinitely against the same already-populated stack is
// always safe. std::pmr::monotonic_buffer_resource has no equivalent
// partial-rollback checkpoint (only whole-resource release()), so
// every case runs solo.
//
// Covers:
// - getMarker() alone
// - freeToMarker() alone, rolling back to an already-captured marker
// - getMarker() immediately followed by freeToMarker() to it — the
//   round trip as StackScope uses it internally

#include <support/framework.h>

#include <benchmark/benchmark.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures getMarker() alone.
static void marker_get(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        auto m = cSrc.getMarker();
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(marker_get);

// Measures freeToMarker() alone, repeatedly rolling back to the same
// previously-captured marker.
static void marker_free(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);
    auto marker = cSrc.getMarker();

    for (auto _ : state) {
        cSrc.freeToMarker(marker);
    }
}
BENCHMARK(marker_free);

// Measures the getMarker() + freeToMarker() round trip together, as
// StackScope performs it internally.
static void marker_roundtrip(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        auto m = cSrc.getMarker();
        cSrc.freeToMarker(m);
    }
}
BENCHMARK(marker_roundtrip);
