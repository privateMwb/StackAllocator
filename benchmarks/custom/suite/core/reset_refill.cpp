// Stack Core Benchmark Suite — Reset & Refill
// Measures Stack reset() performance against stdStack's release(),
// the standard library's own bulk-reclaim operation.
//
// reset() is idempotent — it always sets the offset back to zero
// regardless of prior state — so it can be called repeatedly without
// any per-repetition setup. release() is the same for stdStack: it is
// always safe to call again even with nothing outstanding. The refill
// case additionally performs a small, fixed number of allocations
// after each reset, so every repetition starts from an empty allocator
// and walks back up to the same fixed high-water mark, bounded well
// under the buffer's capacity regardless of iteration count.
//
// Covers:
// - reset() alone, on an allocator with prior allocations outstanding
// - reset() followed by refilling to a fixed number of entries

#include <support/framework.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kRefillEntrySize = sizeof(int);
constexpr std::size_t kRefillEntryCount = 64;
} // namespace

// Measures reset() alone, called repeatedly on an allocator that
// already has allocations outstanding from its initial fill.
static void bench_reset_alone() {
    Stack<false> cSrc(kCapacityBytes);
    stdStack sSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
        doNotOptimize(cSrc.allocate(kRefillEntrySize));
        doNotOptimize(sSrc.allocate(kRefillEntrySize));
    }

    auto c = [&] { cSrc.reset(); };

    auto s = [&] { sSrc.release(); };

    BENCH("reset() alone", c, s);
}

// Measures reset() followed by refilling to a fixed entry count —
// every repetition starts empty and walks back up to the same
// high-water mark.
static void bench_reset_refill() {
    Stack<false> cSrc(kCapacityBytes);
    stdStack sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.reset();
        for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
            doNotOptimize(cSrc.allocate(kRefillEntrySize));
        }
    };

    auto s = [&] {
        sSrc.release();
        for (std::size_t i = 0; i < kRefillEntryCount; ++i) {
            doNotOptimize(sSrc.allocate(kRefillEntrySize));
        }
    };

    BENCH("reset() + refill", c, s);
}

// Executes all reset/refill benchmark cases.
static void run_benchmarks() {
    bench_reset_alone();
    std::cout << "\n";

    bench_reset_refill();
}

REGISTER_BENCH_SUITE();
