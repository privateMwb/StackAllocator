// Stack Core Benchmark Suite — Destroy
// Measures Stack destroy<T>() performance against stdStack,
// the standard library's own linear/bump allocator.
//
// destroy<T>() only runs ~T() — it never reclaims storage. To isolate
// destructor cost from allocation cost, each case reserves one fixed
// block of storage outside the timed loop, then placement-constructs
// and immediately destroys into that same address on every repetition.
// Construction cost is shared identically by both the trivial and
// non-trivial cases, so the measured delta between the two isolates
// destructor-body cost. stdStack has no destroy<T>() of its own, so the
// paired side is built from std::pmr::polymorphic_allocator<T>, the
// standard mechanism for destroying a typed object owned by a
// memory_resource.
//
// Covers:
// - destroy<T>() of a type with a trivial destructor
// - destroy<T>() of a type with a non-trivial destructor

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 4096;

// Small type with a trivial (compiler-generated) destructor.
struct TrivialDtor {
    int value;
};

// Small type with a non-trivial destructor that does real, if
// synthetic, work — no owned resources, so allocation cost stays flat.
struct NonTrivialDtor {
    int values[8];
    ~NonTrivialDtor() noexcept {
        for (int& v : values) {
            benchmark::DoNotOptimize(v);
        }
    }
};
} // namespace

// Measures Stack destroy<T>() of a type with a trivial destructor.
static void destroy_trivial_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);
    TrivialDtor* cPtr = cSrc.create<TrivialDtor>(TrivialDtor{0});

    for (auto _ : state) {
        new (cPtr) TrivialDtor{0};
        cSrc.destroy(cPtr);
    }
}
BENCHMARK(destroy_trivial_stack);

// Measures the polymorphic_allocator equivalent of destroy<T>() for a
// type with a trivial destructor.
static void destroy_trivial_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<TrivialDtor> alloc(&sSrc);

    TrivialDtor* sPtr = alloc.allocate(1);
    alloc.construct(sPtr, TrivialDtor{0});

    for (auto _ : state) {
        new (sPtr) TrivialDtor{0};
        alloc.destroy(sPtr);
    }
}
BENCHMARK(destroy_trivial_std);

// Measures Stack destroy<T>() of a type with a non-trivial destructor.
static void destroy_nontrivial_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    NonTrivialDtor* cPtr = cSrc.create<NonTrivialDtor>();
    cSrc.destroy(cPtr);

    for (auto _ : state) {
        new (cPtr) NonTrivialDtor{};
        cSrc.destroy(cPtr);
    }
}
BENCHMARK(destroy_nontrivial_stack);

// Measures the polymorphic_allocator equivalent of destroy<T>() for a
// type with a non-trivial destructor.
static void destroy_nontrivial_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<NonTrivialDtor> alloc(&sSrc);

    NonTrivialDtor* sPtr = alloc.allocate(1);
    alloc.construct(sPtr);
    alloc.destroy(sPtr);

    for (auto _ : state) {
        new (sPtr) NonTrivialDtor{};
        alloc.destroy(sPtr);
    }
}
BENCHMARK(destroy_nontrivial_std);
