// Stack object lifecycle benchmark suite.
//
// Measures the performance of object construction and destruction
// against equivalent heap allocation scenarios.
//
// Coverage:
// - Create/destroy trivial type vs heap new/delete
// - Create/destroy non-trivial type vs heap new/delete
// - Create with forwarded arguments vs heap new

#include <common/framework.h>

using namespace AllocatorPro;

// Trivial benchmark type.
struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

// Non-trivial benchmark type.
struct Node {
    int         value = 0;
    std::string label;

    explicit Node(int v, std::string s)
        : value{v}
        , label{std::move(s)}
    {}

    ~Node() {}
};

// Measures create/destroy of a trivial type against
// heap allocation and deallocation.
static void bench_create_destroy_trivial() {
    Stack s{1024 * 1024};

    auto stack_trivial = [&] {
        auto marker = s.getMarker();
        Point* ptr  = s.create<Point>();
        doNotOptimize(ptr);
        s.destroy(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_create_destroy_trivial", LARGE, stack_trivial);

    auto heap_trivial = [&] {
        Point* ptr = new Point{};
        doNotOptimize(ptr);
        delete ptr;
        doNotOptimize();
    };
    BENCH("heap_create_destroy_trivial", LARGE, heap_trivial);
}

// Measures create/destroy of a non-trivial type against
// heap allocation and deallocation.
static void bench_create_destroy_nontrivial() {
    Stack s{1024 * 1024};

    auto stack_nontrivial = [&] {
        auto marker = s.getMarker();
        Node* ptr   = s.create<Node>(42, "hello");
        doNotOptimize(ptr);
        s.destroy(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_create_destroy_nontrivial", LARGE, stack_nontrivial);

    auto heap_nontrivial = [&] {
        Node* ptr = new Node{42, "hello"};
        doNotOptimize(ptr);
        delete ptr;
        doNotOptimize();
    };
    BENCH("heap_create_destroy_nontrivial", LARGE, heap_nontrivial);
}

// Measures object construction with forwarded arguments
// against heap allocation.
static void bench_create_with_args() {
    Stack s{1024 * 1024};

    auto stack_args = [&] {
        auto marker = s.getMarker();
        Node* ptr   = s.create<Node>(99, "benchmark");
        doNotOptimize(ptr);
        s.destroy(ptr);
        s.freeToMarker(marker);
        doNotOptimize();
    };
    BENCH("stack_create_with_args", LARGE, stack_args);

    auto heap_args = [&] {
        Node* ptr = new Node{99, "benchmark"};
        doNotOptimize(ptr);
        delete ptr;
        doNotOptimize();
    };
    BENCH("heap_create_with_args", LARGE, heap_args);
}

// Executes all object lifecycle benchmark cases.
static void run_benchmarks() {
    bench_create_destroy_trivial();
    std::cout << "\n";

    bench_create_destroy_nontrivial();
    std::cout << "\n";

    bench_create_with_args();
}

REGISTER_BENCH_SUITE();