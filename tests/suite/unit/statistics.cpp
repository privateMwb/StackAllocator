// Stack statistics test suite.
//
// Coverage:
// - Initial statistics are zero
// - Allocation increments allocation count
// - Allocation updates total allocated
// - Allocation updates current used
// - Allocation updates peak used
// - Peak used does not decrease on free
// - Current used updates on freeToMarker
// - Current used updates on reset
// - Statistics reset on reset

#include <common/framework.h>

using namespace AllocatorPro;

// Verifies that all statistics are initialized to zero.
static void initial_stats_are_zero() {
    Stack<true> s{1024};
    const auto& stats = s.getStats();
    CHK(stats.totalAllocated_ == 0);
    CHK(stats.currentUsed_    == 0);
    CHK(stats.peakUsed_       == 0);
    CHK(stats.allocations_    == 0);
}

// Verifies that each allocation increments the allocation count.
static void allocation_increments_count() {
    Stack<true> s{1024};
    (void)s.allocate(64);
    (void)s.allocate(64);
    CHK(s.getStats().allocations_ == 2);
}

// Verifies that total allocated bytes accumulate across allocations.
static void allocation_updates_total_allocated() {
    Stack<true> s{1024};
    (void)s.allocate(128);
    (void)s.allocate(256);
    CHK(s.getStats().totalAllocated_ == 384);
}

// Verifies that current used bytes reflect the current allocation state.
static void allocation_updates_current_used() {
    Stack<true> s{1024};
    (void)s.allocate(128);
    CHK(s.getStats().currentUsed_ == 128);
}

// Verifies that peak used bytes track the highest allocation level reached.
static void allocation_updates_peak_used() {
    Stack<true> s{1024};
    (void)s.allocate(256);
    (void)s.allocate(256);
    CHK(s.getStats().peakUsed_ == 512);
}

// Verifies that peak used bytes are preserved after freeing memory.
static void peak_used_does_not_decrease_on_free() {
    Stack<true> s{1024};
    auto marker = s.getMarker();
    (void)s.allocate(512);
    s.freeToMarker(marker);
    CHK(s.getStats().peakUsed_ == 512);
}

// Verifies that current used bytes are updated after freeToMarker.
static void current_used_updates_on_free_to_marker() {
    Stack<true> s{1024};
    auto marker = s.getMarker();
    (void)s.allocate(256);
    s.freeToMarker(marker);
    CHK(s.getStats().currentUsed_ == 0);
}

// Verifies that current used bytes are reset to zero.
static void current_used_updates_on_reset() {
    Stack<true> s{1024};
    (void)s.allocate(256);
    s.reset();
    CHK(s.getStats().currentUsed_ == 0);
}

// Verifies that reset clears all runtime statistics.
static void stats_reset_on_reset() {
    Stack<true> s{1024};
    (void)s.allocate(256);
    s.reset();
    const auto& stats = s.getStats();
    CHK(stats.totalAllocated_ == 0);
    CHK(stats.currentUsed_    == 0);
    CHK(stats.peakUsed_       == 0);
    CHK(stats.allocations_    == 0);
}

// Executes all statistics test cases.
static void run_tests() {
    RUN(initial_stats_are_zero);
    RUN(allocation_increments_count);
    RUN(allocation_updates_total_allocated);
    RUN(allocation_updates_current_used);
    RUN(allocation_updates_peak_used);
    RUN(peak_used_does_not_decrease_on_free);
    RUN(current_used_updates_on_free_to_marker);
    RUN(current_used_updates_on_reset);
    RUN(stats_reset_on_reset);
}

REGISTER_TEST_SUITE();