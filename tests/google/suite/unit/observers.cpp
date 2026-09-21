// Stack observers test suite.
//
// Coverage:
// - used() reflects bytes consumed so far
// - remaining() equals capacity() minus used()
// - capacity() stays constant across allocations
// - getStats() reflects allocation count and totals when enabled

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies used() starts at zero and grows by the allocated size.
TEST(Observers, UsedReflectsConsumedBytes) {
    Stack<> stack(64);
    EXPECT_EQ(stack.used(), 0u);

    (void)stack.allocate(16);

    EXPECT_EQ(stack.used(), 16u);
}

// Verifies remaining() is always capacity() minus used().
TEST(Observers, RemainingEqualsCapacityMinusUsed) {
    Stack<> stack(64);
    (void)stack.allocate(16);

    EXPECT_EQ(stack.remaining(), stack.capacity() - stack.used());
}

// Verifies capacity() reflects the buffer size and never changes.
TEST(Observers, CapacityIsConstant) {
    Stack<> stack(64);
    EXPECT_EQ(stack.capacity(), 64u);

    (void)stack.allocate(16);

    EXPECT_EQ(stack.capacity(), 64u);
}

// Verifies getStats() tracks allocation count and byte totals.
TEST(Observers, StatsReflectAllocationsWhenEnabled) {
    Stack<true> stack(64);
    (void)stack.allocate(16);
    (void)stack.allocate(8);

    const auto& stats = stack.getStats();

    EXPECT_EQ(stats.allocations_, 2u);
    EXPECT_EQ(stats.totalAllocated_, 24u);
    EXPECT_EQ(stats.currentUsed_, stack.used());
    EXPECT_EQ(stats.peakUsed_, stack.used());
}
