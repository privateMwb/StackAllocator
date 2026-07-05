// StackScope benchmark suite.
//
// Measures the performance overhead of StackScope against
// manual marker management and equivalent heap allocation scenarios.
//
// Coverage:
// - StackScope vs manual marker management vs heap new/delete
// - Nested StackScope vs nested manual markers vs heap allocation and deallocation

#include <common/framework.h>

using namespace AllocatorPro;

// Measures the overhead of StackScope against
// manual marker management and heap allocation.
static void bench_scope_vs_manual() {
    Stack s{1024 * 1024};

    auto stack_scope = [&] {
        StackScope     scope{s};
        std::byte* ptr = s.allocate(64);
        doNotOptimize(ptr);
        doNotOptimize();
    };
    BENCH("stack_scope", LARGE, stack_scope);

    auto stack_manual = [&] {
        auto marker    = s.getMarker();
        std::byte* ptr = s.allocate(64);
        doNotOptimize(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_manual_marker", LARGE, stack_manual);

    auto heap = [&] {
        void* ptr = ::operator new(64);
        doNotOptimize(ptr);
        ::operator delete(ptr);
        doNotOptimize();
    };
    BENCH("heap_new_delete", LARGE, heap);
}

// Measures nested StackScope usage against
// nested manual marker management and heap allocation.
static void bench_nested_scope_vs_manual() {
    Stack s{1024 * 1024};

    auto stack_nested_scope = [&] {
        StackScope outer{s};
        std::byte* a = s.allocate(64);
        doNotOptimize(a);

        {
            StackScope inner{s};
            std::byte* b = s.allocate(64);
            doNotOptimize(b);
            doNotOptimize();
        }

        doNotOptimize();
    };
    BENCH("stack_nested_scope", LARGE, stack_nested_scope);

    auto stack_nested_manual = [&] {
        auto m0      = s.getMarker();
        std::byte* a = s.allocate(64);
        doNotOptimize(a);

        auto m1      = s.getMarker();
        std::byte* b = s.allocate(64);
        doNotOptimize(b);
        s.freeToMarker(m1);

        s.freeToMarker(m0);
        doNotOptimize();
    };
    BENCH("stack_nested_manual_marker", LARGE, stack_nested_manual);

    auto heap_nested = [&] {
        void* a = ::operator new(64);
        doNotOptimize(a);
        void* b = ::operator new(64);
        doNotOptimize(b);
        ::operator delete(b);
        ::operator delete(a);
        doNotOptimize();
    };
    BENCH("heap_nested_new_delete", LARGE, heap_nested);
}

// Executes all StackScope benchmark cases.
static void run_benchmarks() {
    bench_scope_vs_manual();
    std::cout << "\n";

    bench_nested_scope_vs_manual();
}

REGISTER_BENCH_SUITE();