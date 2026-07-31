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

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures getMarker() alone.
static void bench_get_marker() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    auto c = [&] {
        auto m = cSrc.getMarker();
        doNotOptimize(m);
    };

    BENCH_SOLO("getMarker()", c);
}

// Measures freeToMarker() alone, repeatedly rolling back to the same
// previously-captured marker.
static void bench_free_to_marker() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);
    auto marker = cSrc.getMarker();

    auto c = [&] { cSrc.freeToMarker(marker); };

    BENCH_SOLO("freeToMarker()", c);
}

// Measures the getMarker() + freeToMarker() round trip together, as
// StackScope performs it internally.
static void bench_marker_round_trip() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    auto c = [&] {
        auto m = cSrc.getMarker();
        cSrc.freeToMarker(m);
    };

    BENCH_SOLO("getMarker() + freeToMarker()", c);
}

// Executes all marker/rollback benchmark cases.
static void run_benchmarks() {
    bench_get_marker();
    std::cout << "\n";

    bench_free_to_marker();
    std::cout << "\n";

    bench_marker_round_trip();
}

REGISTER_BENCH_SUITE();
