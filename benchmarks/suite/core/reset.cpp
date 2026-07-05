// Stack reset benchmark suite.
//
// Measures the performance of stack reset operations against
// equivalent heap allocation scenarios.
//
// Coverage:
// - Reset vs heap allocation and deallocation
// - Allocation to exhaustion followed by reset vs heap equivalent
// - Repeated allocation/reset cycle vs heap allocation and deallocation

#include <common/framework.h>

using namespace AllocatorPro;

// Measures the cost of reset against
// heap allocation and deallocation.
static void bench_reset() {
    Stack s{1024 * 1024};

    auto stack_reset = [&] {
        (void)s.allocate(64);
        doNotOptimize();
        s.reset();
        doNotOptimize();
    };
    BENCH("stack_reset", LARGE, stack_reset);

    auto heap_reset = [&] {
        void* ptr = ::operator new(64);
        doNotOptimize(ptr);
        ::operator delete(ptr);
        doNotOptimize();
    };
    BENCH("heap_delete_cycle", LARGE, heap_reset);
}

// Measures allocation to exhaustion followed by reset
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
    BENCH("heap_exhaustion_delete", LARGE, heap_exhaustion);
}

// Measures repeated allocation/reset cycles
// against repeated heap allocation and deallocation.
static void bench_repeated_allocate_reset() {
    Stack s{1024 * 1024};

    auto stack_repeated = [&] {
        std::byte* a = s.allocate(128);
        std::byte* b = s.allocate(128);
        std::byte* c = s.allocate(128);
        doNotOptimize(a);
        doNotOptimize(b);
        doNotOptimize(c);
        s.reset();
        doNotOptimize();
    };
    BENCH("stack_repeated_allocate_reset", LARGE, stack_repeated);

    auto heap_repeated = [&] {
        void* a = ::operator new(128);
        void* b = ::operator new(128);
        void* c = ::operator new(128);
        doNotOptimize(a);
        doNotOptimize(b);
        doNotOptimize(c);
        ::operator delete(a);
        ::operator delete(b);
        ::operator delete(c);
        doNotOptimize();
    };
    BENCH("heap_repeated_new_delete", LARGE, heap_repeated);
}

// Executes all reset benchmark cases.
static void run_benchmarks() {
    bench_reset();
    std::cout << "\n";

    bench_exhaustion_reset();
    std::cout << "\n";

    bench_repeated_allocate_reset();
}

REGISTER_BENCH_SUITE();