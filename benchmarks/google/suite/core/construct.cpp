// Stack Core Benchmark Suite — Construct
// Measures Stack create<T>() performance against stdStack,
// the standard library's own linear/bump allocator.
//
// Each case owns a buffer sized generously above what its fixed
// iteration count can consume, so repeated calls never exhaust capacity
// mid-benchmark — only the steady-state allocate-plus-construct path is
// measured. Iteration counts are pinned (rather than left to Google
// Benchmark's adaptive scaling) precisely to guarantee that bound.
// stdStack has no create<T>() of its own, so the paired side is built
// from std::pmr::polymorphic_allocator<T>, the standard mechanism for
// allocating and constructing a typed object from a memory_resource.
//
// Covers:
// - create<T>() of a small type with a trivial constructor
// - create<T>() of a small type with a non-trivial, multi-argument
//   constructor

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <memory_resource>

using namespace StackPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;

// Worst case: 1,000,000 x sizeof(Vec3) (24 B) = 24 MB < kCapacityBytes.
constexpr benchmark::IterationCount kIterations = 1'000'000;

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

// Measures Stack create<T>() of a small type with a trivial constructor.
static void construct_trivial_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        Trivial* p = cSrc.create<Trivial>(Trivial{42});
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(construct_trivial_stack)->Iterations(kIterations);

// Measures the polymorphic_allocator equivalent of create<T>() for a
// small type with a trivial constructor.
static void construct_trivial_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<Trivial> alloc(&sSrc);

    for (auto _ : state) {
        Trivial* p = alloc.allocate(1);
        alloc.construct(p, Trivial{42});
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(construct_trivial_std)->Iterations(kIterations);

// Measures Stack create<T>() of a small type with a non-trivial,
// multi-argument constructor.
static void construct_nontrivial_stack(benchmark::State& state) {
    Stack<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        Vec3* p = cSrc.create<Vec3>(1.0, 2.0, 3.0);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(construct_nontrivial_stack)->Iterations(kIterations);

// Measures the polymorphic_allocator equivalent of create<T>() for a
// small type with a non-trivial, multi-argument constructor.
static void construct_nontrivial_std(benchmark::State& state) {
    stdStack sSrc(kCapacityBytes);
    std::pmr::polymorphic_allocator<Vec3> alloc(&sSrc);

    for (auto _ : state) {
        Vec3* p = alloc.allocate(1);
        alloc.construct(p, 1.0, 2.0, 3.0);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(construct_nontrivial_std)->Iterations(kIterations);
