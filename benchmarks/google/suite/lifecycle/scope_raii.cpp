// Lifecycle Benchmark Suite — Scope RAII
// Measures StackScope's constructor/destructor cost.
//
// StackScope captures a marker on construction and rolls the stack
// back to it on destruction — a thin wrapper around
// getMarker()/freeToMarker(). Nothing is allocated inside the scope
// here, so the stack's state never changes across repetitions, and
// the pattern is safe to repeat indefinitely. std::pmr::
// monotonic_buffer_resource has no equivalent scoped-checkpoint type,
// so this runs solo.
//
// Covers:
// - constructing and destroying a StackScope over an
//   already-populated stack

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <StackPro/StackScope.h>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
constexpr std::size_t kUsed = 256;
} // namespace

// Measures a StackScope's construction immediately followed by its
// destruction.
static void scope_raii(benchmark::State& state) {
    Stack<false> cSrc(kSize);
    (void)cSrc.allocate(kUsed);

    for (auto _ : state) {
        StackScope<false> scope(cSrc);
        benchmark::DoNotOptimize(&scope);
    }
}
BENCHMARK(scope_raii);
