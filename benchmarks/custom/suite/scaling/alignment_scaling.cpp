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

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
constexpr std::size_t kSmallAlignment = 4;
constexpr std::size_t kMediumAlignment = 64;
constexpr std::size_t kLargeAlignment = 4096;
} // namespace

// Measures allocate() at 4-byte alignment.
static void bench_alignment_4() {
    Stack<false> cSrc(kCapacityBytes, kSmallAlignment);
    stdStack sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.reset();
        doNotOptimize(cSrc.allocate(kAllocSize, kSmallAlignment));
    };

    auto s = [&] {
        sSrc.release();
        doNotOptimize(sSrc.allocate(kAllocSize, kSmallAlignment));
    };

    BENCH("alloc 4b align", c, s);
}

// Measures allocate() at 64-byte alignment.
static void bench_alignment_64() {
    Stack<false> cSrc(kCapacityBytes, kMediumAlignment);
    stdStack sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.reset();
        doNotOptimize(cSrc.allocate(kAllocSize, kMediumAlignment));
    };

    auto s = [&] {
        sSrc.release();
        doNotOptimize(sSrc.allocate(kAllocSize, kMediumAlignment));
    };

    BENCH("alloc 64b align", c, s);
}

// Measures allocate() at 4096-byte alignment.
static void bench_alignment_4096() {
    Stack<false> cSrc(kCapacityBytes, kLargeAlignment);
    stdStack sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.reset();
        doNotOptimize(cSrc.allocate(kAllocSize, kLargeAlignment));
    };

    auto s = [&] {
        sSrc.release();
        doNotOptimize(sSrc.allocate(kAllocSize, kLargeAlignment));
    };

    BENCH("alloc 4096b align", c, s);
}

// Executes all alignment scaling benchmark cases.
static void run_benchmarks() {
    bench_alignment_4();
    bench_alignment_64();
    bench_alignment_4096();
}

REGISTER_BENCH_SUITE();
