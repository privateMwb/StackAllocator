// Stack reset() test suite.
//
// Coverage:
// - Rewinds used() back to zero
// - Leaves capacity() unchanged
// - Clears accumulated statistics when stats are enabled

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies reset() rewinds the used offset to zero.
TEST(Reset, ResetsOffsetToZero) {
    Stack<> stack(64);
    (void)stack.allocate(16);

    stack.reset();

    EXPECT_EQ(stack.used(), 0u);
}

// Verifies reset() does not shrink or grow the underlying buffer.
TEST(Reset, LeavesCapacityUnchanged) {
    Stack<> stack(64);
    (void)stack.allocate(16);

    stack.reset();

    EXPECT_EQ(stack.capacity(), 64u);
}

// Verifies reset() zeroes out accumulated stats when EnableStats is true.
TEST(Reset, ClearsStatsWhenEnabled) {
    Stack<true> stack(64);
    (void)stack.allocate(16);

    stack.reset();
    const auto& stats = stack.getStats();

    EXPECT_EQ(stats.currentUsed_, 0u);
    EXPECT_EQ(stats.peakUsed_, 0u);
    EXPECT_EQ(stats.allocations_, 0u);
    EXPECT_EQ(stats.totalAllocated_, 0u);
}
