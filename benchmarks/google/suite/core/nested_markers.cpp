// Core Benchmark Suite — Nested Markers
// Measures the cost of a nested checkpoint/rollback chain: capturing
// several markers in sequence, each after an allocation, then
// unwinding them in reverse order — the pattern nested StackScopes
// produce in real call stacks.
//
// The marker vector is reserved once, outside the timed loop, and
// only cleared (not reallocated) inside it, so no heap allocation
// leaks into the measurement. The stack itself is reset at the start
// of every repetition so the fixed-depth chain can be replayed an
// unbounded number of times without ever exhausting capacity.
// std::pmr::monotonic_buffer_resource has no checkpoint concept at
// all, so this runs solo.
//
// Covers:
// - building and unwinding a depth-8 chain of nested markers

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <vector>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kEntrySize = sizeof(int);
constexpr int kDepth = 8;
} // namespace

// Measures a full build-up and unwind of a depth-8 nested marker chain.
static void nested_markers_depth8(benchmark::State& state) {
    Stack<false> cSrc(kSize);

    std::vector<Stack<false>::Marker> markers;
    markers.reserve(kDepth);

    for (auto _ : state) {
        cSrc.reset();
        markers.clear(); // Keeps reserved capacity; no reallocation.

        for (int i = 0; i < kDepth; ++i) {
            markers.push_back(cSrc.getMarker());
            benchmark::DoNotOptimize(cSrc.allocate(kEntrySize));
        }

        for (auto it = markers.rbegin(); it != markers.rend(); ++it) {
            cSrc.freeToMarker(*it);
        }
    }
}
BENCHMARK(nested_markers_depth8);
