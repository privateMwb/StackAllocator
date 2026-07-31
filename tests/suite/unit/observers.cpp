// Stack observers test suite.
//
// Coverage:
// - used() reflects bytes consumed so far
// - remaining() equals capacity() minus used()
// - capacity() stays constant across allocations
// - getStats() reflects allocation count and totals when enabled

#include <support/framework.h>

using namespace StackPro;

// Verifies used() starts at zero and grows by the allocated size.
static void used_reflects_consumed_bytes() {
    Stack<> stack(64);
    CHK(stack.used() == 0);

    (void)stack.allocate(16);

    CHK(stack.used() == 16);
}

// Verifies remaining() is always capacity() minus used().
static void remaining_equals_capacity_minus_used() {
    Stack<> stack(64);
    (void)stack.allocate(16);

    CHK(stack.remaining() == stack.capacity() - stack.used());
}

// Verifies capacity() reflects the buffer size and never changes.
static void capacity_is_constant() {
    Stack<> stack(64);
    CHK(stack.capacity() == 64);

    (void)stack.allocate(16);

    CHK(stack.capacity() == 64);
}

// Verifies getStats() tracks allocation count and byte totals.
static void stats_reflect_allocations_when_enabled() {
    Stack<true> stack(64);
    (void)stack.allocate(16);
    (void)stack.allocate(8);

    const auto& stats = stack.getStats();

    CHK(stats.allocations_ == 2);
    CHK(stats.totalAllocated_ == 24);
    CHK(stats.currentUsed_ == stack.used());
    CHK(stats.peakUsed_ == stack.used());
}

// Executes all observer test cases.
static void run_tests() {
    RUN(used_reflects_consumed_bytes);
    RUN(remaining_equals_capacity_minus_used);
    RUN(capacity_is_constant);
    RUN(stats_reflect_allocations_when_enabled);
}

REGISTER_TEST_SUITE();
