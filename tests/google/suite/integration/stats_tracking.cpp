// Stack stats tracking integration suite.
//
// Coverage:
// - Lifetime totals (allocations_, totalAllocated_) survive a scope
//   rollback, even though currentUsed_ drops back down
// - peakUsed_ records the high-water mark and isn't lowered by a
//   later freeToMarker() and smaller re-allocation
// - totalAllocated_ and allocations_ accumulate across separate calls

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies lifetime stats persist through a scope rollback, while
// currentUsed_ reflects the rolled-back offset.
TEST(StatsTracking, StatsSurviveScopeRollback) {
    Stack<true> stack(64);

    {
        StackScope<true> scope(stack);
        (void)stack.allocate(16);
        (void)stack.allocate(8);
    }

    const auto& stats = stack.getStats();
    EXPECT_EQ(stats.allocations_, 2u);
    EXPECT_EQ(stats.totalAllocated_, 24u);
    EXPECT_EQ(stats.currentUsed_, 0u);
    EXPECT_EQ(stats.peakUsed_, 24u);
}

// Verifies peakUsed_ keeps the high-water mark even after a rollback
// followed by a smaller allocation.
TEST(StatsTracking, PeakUsedTracksHighWaterMark) {
    Stack<true> stack(64);
    (void)stack.allocate(32);
    auto marker = stack.getMarker();

    (void)stack.allocate(16);   // used = 48, new peak
    stack.freeToMarker(marker); // used back down to 32
    (void)stack.allocate(4);    // used = 36, below the earlier peak

    const auto& stats = stack.getStats();
    EXPECT_EQ(stats.peakUsed_, 48u);
    EXPECT_EQ(stats.currentUsed_, 36u);
}

// Verifies totalAllocated_ and allocations_ accumulate across
// independent allocate() calls.
TEST(StatsTracking, TotalAllocatedAccumulatesAcrossCalls) {
    Stack<true> stack(64);
    (void)stack.allocate(8);
    (void)stack.allocate(8);
    (void)stack.allocate(8);

    const auto& stats = stack.getStats();
    EXPECT_EQ(stats.totalAllocated_, 24u);
    EXPECT_EQ(stats.allocations_, 3u);
}
