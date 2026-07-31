// Lifecycle Benchmark Suite — Move Semantics
// Measures Stack's move-construction and move-assignment cost.
//
// The source (and, for move-assign, destination) stack is built once,
// outside the timed lambda, with a real backing allocation. Only the
// first repetition actually transfers and frees that real buffer;
// every repetition after it moves an already-empty (moved-from)
// stack, which is safe to repeat indefinitely since Stack's move
// operations are a fixed sequence of pointer/field exchanges that
// don't branch on whether the source is null — so steady-state timing
// is representative regardless. std::pmr::monotonic_buffer_resource is
// neither copyable nor movable, so it has no equivalent here and both
// cases run solo.
//
// Covers:
// - move-constructing a Stack from another (including the resulting
//   temporary's destruction, since a freshly move-constructed local
//   necessarily goes out of scope at the end of each repetition)
// - move-assigning a Stack from another

#include <support/framework.h>

#include <utility>

using namespace StackPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures move-construction, plus the resulting temporary's
// destruction at the end of each repetition.
static void bench_move_construct() {
    Stack<false> src(kSize);

    auto c = [&] {
        Stack<false> dst(std::move(src));
        doNotOptimize(&dst);
    };

    BENCH_SOLO("move construct", c);
}

// Measures move-assignment from src into an already-constructed dst.
static void bench_move_assign() {
    Stack<false> src(kSize);
    Stack<false> dst(kSize);

    auto c = [&] {
        dst = std::move(src);
        doNotOptimize(&dst);
    };

    BENCH_SOLO("move assign", c);
}

// Executes all move semantics benchmark cases.
static void run_benchmarks() {
    bench_move_construct();
    std::cout << "\n";

    bench_move_assign();
}

REGISTER_BENCH_SUITE();
