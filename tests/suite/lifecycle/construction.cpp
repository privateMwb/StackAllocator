// Stack construction test suite.
//
// Coverage:
// - A freshly constructed stack has zero used bytes and full remaining
//   capacity
// - capacity() matches the size requested at construction exactly
// - Statistics start at all-zero when EnableStats is true

#include <support/framework.h>

using namespace StackPro;

// Verifies a fresh stack starts with nothing allocated.
static void fresh_stack_starts_empty() {
    Stack<> stack(64);

    CHK(stack.used() == 0);
    CHK(stack.remaining() == 64);
}

// Verifies capacity() reports exactly the size requested at construction.
static void capacity_matches_requested_size() {
    Stack<> stack(100);
    CHK(stack.capacity() == 100);
}

// Verifies statistics are all zero immediately after construction.
static void stats_start_at_zero_when_enabled() {
    Stack<true> stack(64);
    const auto& stats = stack.getStats();

    CHK(stats.totalAllocated_ == 0);
    CHK(stats.currentUsed_ == 0);
    CHK(stats.peakUsed_ == 0);
    CHK(stats.allocations_ == 0);
}

// Executes all construction test cases.
static void run_tests() {
    RUN(fresh_stack_starts_empty);
    RUN(capacity_matches_requested_size);
    RUN(stats_start_at_zero_when_enabled);
}

REGISTER_TEST_SUITE();
