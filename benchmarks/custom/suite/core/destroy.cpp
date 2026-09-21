// Stack Core Benchmark Suite — Destroy
// Measures Stack destroy<T>() performance against stdStack,
// the standard library's own linear/bump allocator.
//
// destroy<T>() only runs ~T() — it never reclaims storage. To isolate
// destructor cost from allocation cost, each case reserves one fixed
// block of storage outside the timed lambda, then placement-constructs
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
            doNotOptimize(v);
        }
    }
};
} // namespace

// Measures destroy<T>() of a type with a trivial destructor.
static void bench_destroy_trivial() {
    Stack<false> cSrc(kCapacityBytes);
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<TrivialDtor> alloc(&sSrc);

    TrivialDtor* cPtr = cSrc.create<TrivialDtor>(TrivialDtor{0});
    TrivialDtor* sPtr = alloc.allocate(1);
    alloc.construct(sPtr, TrivialDtor{0});

    auto c = [&] {
        new (cPtr) TrivialDtor{0};
        cSrc.destroy(cPtr);
    };

    auto s = [&] {
        new (sPtr) TrivialDtor{0};
        alloc.destroy(sPtr);
    };

    BENCH("destroy trivial dtor", c, s);
}

// Measures destroy<T>() of a type with a non-trivial destructor.
static void bench_destroy_nontrivial() {
    Stack<false> cSrc(kCapacityBytes);
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<NonTrivialDtor> alloc(&sSrc);

    NonTrivialDtor* cPtr = cSrc.create<NonTrivialDtor>();
    NonTrivialDtor* sPtr = alloc.allocate(1);
    alloc.construct(sPtr);
    cSrc.destroy(cPtr);
    alloc.destroy(sPtr);

    auto c = [&] {
        new (cPtr) NonTrivialDtor{};
        cSrc.destroy(cPtr);
    };

    auto s = [&] {
        new (sPtr) NonTrivialDtor{};
        alloc.destroy(sPtr);
    };

    BENCH("destroy nontrivial dtor", c, s);
}

// Executes all destroy benchmark cases.
static void run_benchmarks() {
    bench_destroy_trivial();
    bench_destroy_nontrivial();
}

REGISTER_BENCH_SUITE();
