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
// Covers:
// - allocate() with room to spare
// - allocate() once the allocator is completely full

#include <support/framework.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kHeadroomCapacity = 64 * 1024 * 1024;

// Must be large enough to satisfy one allocation at the default
// alignment (alignof(std::max_align_t)) with zero bytes left over —
// otherwise the very first (untimed) setup allocation falls through
// to the upstream null_memory_resource() and throws before the timed
// loop ever starts.
constexpr std::size_t kFullCapacity = alignof(std::max_align_t);
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures allocate() with plenty of remaining capacity — the
// steady-state success path.
static void bench_exhaustion_headroom() {
    Stack<false> cSrc(kHeadroomCapacity);
    stdStack sSrc(kHeadroomCapacity);

    auto c = [&] { doNotOptimize(cSrc.allocate(kAllocSize)); };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize)); };

    BENCH("alloc w/ headroom", c, s);
}

// Measures allocate() once the allocator is completely full. Stack
// returns nullptr with no state change, so the call is safe to repeat
// indefinitely; stdStack, bounded with a null_memory_resource()
// upstream, must throw std::bad_alloc on every call instead.
static void bench_exhaustion_full() {
    Stack<false> cSrc(kFullCapacity);
    doNotOptimize(cSrc.allocate(kFullCapacity));

    alignas(std::max_align_t) std::byte buffer[kFullCapacity];
    stdStack sSrc(buffer, sizeof(buffer), std::pmr::null_memory_resource());

    // This must succeed exactly, consuming the entire buffer with
    // nothing left over. If it throws here, the buffer above is sized
    // or aligned incorrectly for this standard library implementation.
    try {
        doNotOptimize(sSrc.allocate(kFullCapacity));
    } catch (const std::bad_alloc&) {
        AP_ASSERT(false && "setup allocation should never fail here");
        throw;
    }

    auto c = [&] { doNotOptimize(cSrc.allocate(kAllocSize)); };

    auto s = [&] {
        try {
            doNotOptimize(sSrc.allocate(kAllocSize));
        } catch (const std::bad_alloc&) {
            // Expected: upstream is null_memory_resource().
        }
    };

    BENCH("alloc at capacity", c, s);
}

// Executes all exhaustion benchmark cases.
static void run_benchmarks() {
    bench_exhaustion_headroom();
    bench_exhaustion_full();
}

REGISTER_BENCH_SUITE();
