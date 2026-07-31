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

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kSmallCapacity = 4 * 1024;
constexpr std::size_t kMediumCapacity = 1024 * 1024;
constexpr std::size_t kLargeCapacity = 64 * 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures allocate() against a 4 KiB buffer.
static void bench_capacity_4kib() {
    Stack<false> cSrc(kSmallCapacity);
    stdStack sSrc(kSmallCapacity);

    auto c = [&] {
        cSrc.reset();
        doNotOptimize(cSrc.allocate(kAllocSize));
    };

    auto s = [&] {
        sSrc.release();
        doNotOptimize(sSrc.allocate(kAllocSize));
    };

    BENCH("allocate() @ 4 KiB", c, s);
}

// Measures allocate() against a 1 MiB buffer.
static void bench_capacity_1mib() {
    Stack<false> cSrc(kMediumCapacity);
    stdStack sSrc(kMediumCapacity);

    auto c = [&] {
        cSrc.reset();
        doNotOptimize(cSrc.allocate(kAllocSize));
    };

    auto s = [&] {
        sSrc.release();
        doNotOptimize(sSrc.allocate(kAllocSize));
    };

    BENCH("allocate() @ 1 MiB", c, s);
}

// Measures allocate() against a 64 MiB buffer.
static void bench_capacity_64mib() {
    Stack<false> cSrc(kLargeCapacity);
    stdStack sSrc(kLargeCapacity);

    auto c = [&] {
        cSrc.reset();
        doNotOptimize(cSrc.allocate(kAllocSize));
    };

    auto s = [&] {
        sSrc.release();
        doNotOptimize(sSrc.allocate(kAllocSize));
    };

    BENCH("allocate() @ 64 MiB", c, s);
}

// Executes all capacity growth benchmark cases.
static void run_benchmarks() {
    bench_capacity_4kib();
    std::cout << "\n";

    bench_capacity_1mib();
    std::cout << "\n";

    bench_capacity_64mib();
}

REGISTER_BENCH_SUITE();
