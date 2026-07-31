// Stack reset() test suite.
//
// Coverage:
// - Rewinds used() back to zero
// - Leaves capacity() unchanged
// - Clears accumulated statistics when stats are enabled

#include <support/framework.h>

using namespace StackPro;

// Verifies reset() rewinds the used offset to zero.
static void resets_offset_to_zero() {
    Stack<> stack(64);
    (void)stack.allocate(16);

    stack.reset();

    CHK(stack.used() == 0);
}

// Verifies reset() does not shrink or grow the underlying buffer.
static void leaves_capacity_unchanged() {
    Stack<> stack(64);
    (void)stack.allocate(16);

    stack.reset();

    CHK(stack.capacity() == 64);
}

// Verifies reset() zeroes out accumulated stats when EnableStats is true.
static void clears_stats_when_enabled() {
    Stack<true> stack(64);
    (void)stack.allocate(16);

    stack.reset();
    const auto& stats = stack.getStats();

    CHK(stats.currentUsed_ == 0);
    CHK(stats.peakUsed_ == 0);
    CHK(stats.allocations_ == 0);
    CHK(stats.totalAllocated_ == 0);
}

// Executes all reset() test cases.
static void run_tests() {
    RUN(resets_offset_to_zero);
    RUN(leaves_capacity_unchanged);
    RUN(clears_stats_when_enabled);
}

REGISTER_TEST_SUITE();
