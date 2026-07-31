// Stack Core Benchmark Suite — Construct
// Measures Stack create<T>() performance against stdStack,
// the standard library's own linear/bump allocator.
//
// Each case owns a buffer sized generously above the LARGE iteration
// tier, so repeated calls never exhaust capacity mid-benchmark — only
// the steady-state allocate-plus-construct path is measured. stdStack
// has no create<T>() of its own, so the paired side is built from
// std::pmr::polymorphic_allocator<T>, the standard mechanism for
// allocating and constructing a typed object from a memory_resource.
//
// Covers:
// - create<T>() of a small type with a trivial constructor
// - create<T>() of a small type with a non-trivial, multi-argument
//   constructor

#include <support/framework.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;

// Small type with a trivial (compiler-generated) constructor.
struct Trivial {
    int value;
};

// Small type with a non-trivial, multi-argument constructor, but no
// owned resources — isolates constructor-body cost from allocation cost.
struct Vec3 {
    double x, y, z;
    Vec3(double x_, double y_, double z_) noexcept : x(x_), y(y_), z(z_) {}
};
} // namespace

// Measures create<T>() of a small type with a trivial constructor.
static void bench_construct_trivial() {
    Stack<false> cSrc(kCapacityBytes);
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<Trivial> alloc(&sSrc);

    auto c = [&] {
        Trivial* p = cSrc.create<Trivial>(Trivial{42});
        doNotOptimize(p);
    };

    auto s = [&] {
        Trivial* p = alloc.allocate(1);
        alloc.construct(p, Trivial{42});
        doNotOptimize(p);
    };

    BENCH("create<T>() trivial ctor", c, s);
}

// Measures create<T>() of a small type with a non-trivial,
// multi-argument constructor.
static void bench_construct_nontrivial() {
    Stack<false> cSrc(kCapacityBytes);
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<Vec3> alloc(&sSrc);

    auto c = [&] {
        Vec3* p = cSrc.create<Vec3>(1.0, 2.0, 3.0);
        doNotOptimize(p);
    };

    auto s = [&] {
        Vec3* p = alloc.allocate(1);
        alloc.construct(p, 1.0, 2.0, 3.0);
        doNotOptimize(p);
    };

    BENCH("create<T>() non-trivial ctor", c, s);
}

// Executes all construct benchmark cases.
static void run_benchmarks() {
    bench_construct_trivial();
    std::cout << "\n";

    bench_construct_nontrivial();
}

REGISTER_BENCH_SUITE();
