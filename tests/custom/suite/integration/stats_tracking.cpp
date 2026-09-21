// Stack stats tracking integration suite.
//
// Coverage:
// - Lifetime totals (allocations_, totalAllocated_) survive a scope
//   rollback, even though currentUsed_ drops back down
// - peakUsed_ records the high-water mark and isn't lowered by a
//   later freeToMarker() and smaller re-allocation
// - totalAllocated_ and allocations_ accumulate across separate calls

#include <support/framework.h>

using namespace StackPro;

// Verifies lifetime stats persist through a scope rollback, while
// currentUsed_ reflects the rolled-back offset.
static void stats_survive_scope_rollback() {
    Stack<true> stack(64);

    {
        StackScope<true> scope(stack);
        (void)stack.allocate(16);
        (void)stack.allocate(8);
    }

    const auto& stats = stack.getStats();
    CHK(stats.allocations_ == 2);
    CHK(stats.totalAllocated_ == 24);
    CHK(stats.currentUsed_ == 0);
    CHK(stats.peakUsed_ == 24);
}

// Verifies peakUsed_ keeps the high-water mark even after a rollback
// followed by a smaller allocation.
static void peak_used_tracks_high_water_mark() {
    Stack<true> stack(64);
    (void)stack.allocate(32);
    auto marker = stack.getMarker();

    (void)stack.allocate(16);   // used = 48, new peak
    stack.freeToMarker(marker); // used back down to 32
    (void)stack.allocate(4);    // used = 36, below the earlier peak

    const auto& stats = stack.getStats();
    CHK(stats.peakUsed_ == 48);
    CHK(stats.currentUsed_ == 36);
}

// Verifies totalAllocated_ and allocations_ accumulate across
// independent allocate() calls.
static void total_allocated_accumulates_across_calls() {
    Stack<true> stack(64);
    (void)stack.allocate(8);
    (void)stack.allocate(8);
    (void)stack.allocate(8);

    const auto& stats = stack.getStats();
    CHK(stats.totalAllocated_ == 24);
    CHK(stats.allocations_ == 3);
}

// Executes all stats tracking test cases.
static void run_tests() {
    RUN(stats_survive_scope_rollback);
    RUN(peak_used_tracks_high_water_mark);
    RUN(total_allocated_accumulates_across_calls);
}

REGISTER_TEST_SUITE();
