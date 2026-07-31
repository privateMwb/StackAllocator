// Access Benchmark Suite — Ownership
// Measures Stack owns() performance.
//
// Each case builds its source stack once, outside the timed lambda —
// only repeated ownership checks against an already-populated stack
// are measured. std::pmr::monotonic_buffer_resource exposes no
// ownership query, so both cases run solo.
//
// Covers:
// - owns() on a pointer that belongs to the stack (hit path)
// - owns() on a pointer that does not belong to the stack (miss path)

#include <support/framework.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures owns() on a pointer allocated from the stack.
static void bench_owns_hit() {
    Stack<false> cSrc(kSize);
    std::byte* ptr = cSrc.allocate(sizeof(int));

    auto c = [&] {
        bool v = cSrc.owns(ptr);
        doNotOptimize(v);
    };

    BENCH_SOLO("owns() hit", c);
}

// Measures owns() on a pointer that does not belong to the stack.
static void bench_owns_miss() {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(sizeof(int));

    int outside = 0;
    const void* ptr = &outside;

    auto c = [&] {
        bool v = cSrc.owns(ptr);
        doNotOptimize(v);
    };

    BENCH_SOLO("owns() miss", c);
}

// Executes all ownership benchmark cases.
static void run_benchmarks() {
    bench_owns_hit();
    std::cout << "\n";

    bench_owns_miss();
}

REGISTER_BENCH_SUITE();
