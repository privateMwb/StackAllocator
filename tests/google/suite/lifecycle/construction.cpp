// Stack construction test suite.
//
// Coverage:
// - A freshly constructed stack has zero used bytes and full remaining
//   capacity
// - capacity() matches the size requested at construction exactly
// - Statistics start at all-zero when EnableStats is true

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies a fresh stack starts with nothing allocated.
TEST(Construction, FreshStackStartsEmpty) {
    Stack<> stack(64);

    EXPECT_EQ(stack.used(), 0u);
    EXPECT_EQ(stack.remaining(), 64u);
}

// Verifies capacity() reports exactly the size requested at construction.
TEST(Construction, CapacityMatchesRequestedSize) {
    Stack<> stack(100);
    EXPECT_EQ(stack.capacity(), 100u);
}

// Verifies statistics are all zero immediately after construction.
TEST(Construction, StatsStartAtZeroWhenEnabled) {
    Stack<true> stack(64);
    const auto& stats = stack.getStats();

    EXPECT_EQ(stats.totalAllocated_, 0u);
    EXPECT_EQ(stats.currentUsed_, 0u);
    EXPECT_EQ(stats.peakUsed_, 0u);
    EXPECT_EQ(stats.allocations_, 0u);
}
