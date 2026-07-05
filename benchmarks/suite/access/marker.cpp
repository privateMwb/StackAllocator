// Stack marker benchmark suite.
//
// Measures the performance of marker capture and restoration
// against equivalent heap allocation scenarios.
//
// Coverage:
// - Marker capture and restoration vs heap new/delete
// - Nested marker pattern vs heap allocation and deallocation

#include <common/framework.h>

using namespace AllocatorPro;

// Measures the cost of a marker capture and restoration
// against heap allocation and deallocation.
static void bench_marker_round_trip() {
    Stack s{1024 * 1024};

    auto stack_marker = [&] {
        auto marker    = s.getMarker();
        std::byte* ptr = s.allocate(64);
        doNotOptimize(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_marker_round_trip", LARGE, stack_marker);

    auto heap_marker = [&] {
        void* ptr = ::operator new(64);
        doNotOptimize(ptr);
        ::operator delete(ptr);
        doNotOptimize();
    };
    BENCH("heap_marker_round_trip", LARGE, heap_marker);
}

// Measures nested marker capture and restoration
// against an equivalent heap allocation pattern.
static void bench_nested_markers() {
    Stack s{1024 * 1024};

    auto stack_nested = [&] {
        auto m0        = s.getMarker();
        std::byte* a   = s.allocate(64);
        doNotOptimize(a);

        auto m1        = s.getMarker();
        std::byte* b   = s.allocate(64);
        doNotOptimize(b);

        auto m2        = s.getMarker();
        std::byte* c   = s.allocate(64);
        doNotOptimize(c);

        s.freeToMarker(m2);
        s.freeToMarker(m1);
        s.freeToMarker(m0);
        doNotOptimize();
    };
    BENCH("stack_nested_markers", LARGE, stack_nested);

    auto heap_nested = [&] {
        void* a = ::operator new(64);
        doNotOptimize(a);
        void* b = ::operator new(64);
        doNotOptimize(b);
        void* c = ::operator new(64);
        doNotOptimize(c);
        ::operator delete(c);
        ::operator delete(b);
        ::operator delete(a);
        doNotOptimize();
    };
    BENCH("heap_nested_markers", LARGE, heap_nested);
}

// Executes all marker benchmark cases.
static void run_benchmarks() {
    bench_marker_round_trip();
    std::cout << "\n";

    bench_nested_markers();
}

REGISTER_BENCH_SUITE();