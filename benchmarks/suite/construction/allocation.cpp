// Stack allocation benchmark suite.
//
// Measures the performance of stack allocation against equivalent
// heap allocation scenarios.
//
// Coverage:
// - Single allocation and marker restore vs heap new/delete
// - Aligned allocation vs aligned heap new/delete
// - Sequential allocations vs heap new/delete
// - Allocation to exhaustion followed by reset vs heap equivalent

#include <common/framework.h>

using namespace AllocatorPro;

// Measures the cost of a single stack allocation and marker restore
// against heap allocation and deallocation.
static void bench_allocate() {
    Stack s{1024 * 1024};

    auto stack_allocate = [&] {
        auto marker     = s.getMarker();
        std::byte* ptr  = s.allocate(64);
        doNotOptimize(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_allocate", LARGE, stack_allocate);

    auto heap_allocate = [&] {
        void* ptr = ::operator new(64);
        doNotOptimize(ptr);
        ::operator delete(ptr);
        doNotOptimize();
    };
    BENCH("heap_allocate", LARGE, heap_allocate);
}

// Measures the overhead of aligned stack allocation against
// aligned heap allocation.
static void bench_allocate_aligned() {
    Stack s{1024 * 1024};

    auto stack_aligned = [&] {
        auto marker     = s.getMarker();
        std::byte* ptr  = s.allocate(64, 64);
        doNotOptimize(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_allocate_aligned", LARGE, stack_aligned);

    auto heap_aligned = [&] {
        void* ptr = ::operator new(64, std::align_val_t{64});
        doNotOptimize(ptr);
        ::operator delete(ptr, std::align_val_t{64});
        doNotOptimize();
    };
    BENCH("heap_allocate_aligned", LARGE, heap_aligned);
}

// Measures the cost of multiple sequential stack allocations
// against heap allocation and deallocation.
static void bench_sequential() {
    Stack s{1024 * 1024};

    auto stack_sequential = [&] {
        auto marker    = s.getMarker();
        std::byte* a   = s.allocate(64);
        std::byte* b   = s.allocate(64);
        std::byte* c   = s.allocate(64);
        doNotOptimize(a);
        doNotOptimize(b);
        doNotOptimize(c);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_sequential", LARGE, stack_sequential);

    auto heap_sequential = [&] {
        void* a = ::operator new(64);
        void* b = ::operator new(64);
        void* c = ::operator new(64);
        doNotOptimize(a);
        doNotOptimize(b);
        doNotOptimize(c);
        ::operator delete(a);
        ::operator delete(b);
        ::operator delete(c);
        doNotOptimize();
    };
    BENCH("heap_sequential", LARGE, heap_sequential);
}

// Measures repeated full allocation cycles using reset
// against an equivalent heap allocation pattern.
static void bench_exhaustion_reset() {
    constexpr std::size_t blockSize  = 64;
    constexpr std::size_t blockCount = 16;
    constexpr std::size_t poolSize   = blockSize * blockCount;

    Stack s{poolSize};

    auto stack_exhaustion = [&] {
        for (std::size_t i = 0; i < blockCount; ++i) {
            std::byte* ptr = s.allocate(blockSize);
            doNotOptimize(ptr);
        }
        s.reset();
        doNotOptimize();
    };
    BENCH("stack_exhaustion_reset", LARGE, stack_exhaustion);

    auto heap_exhaustion = [&] {
        void* ptrs[blockCount];
        for (std::size_t i = 0; i < blockCount; ++i) {
            ptrs[i] = ::operator new(blockSize);
            doNotOptimize(ptrs[i]);
        }
        for (std::size_t i = 0; i < blockCount; ++i) {
            ::operator delete(ptrs[i]);
        }
        doNotOptimize();
    };
    BENCH("heap_exhaustion_reset", LARGE, heap_exhaustion);
}

// Executes all allocation benchmark cases.
static void run_benchmarks() {
    bench_allocate();
    std::cout << "\n";

    bench_allocate_aligned();
    std::cout << "\n";

    bench_sequential();
    std::cout << "\n";

    bench_exhaustion_reset();
}

REGISTER_BENCH_SUITE();