// Allocation statistics.
//
// Demonstrates:
// - Stack<true>'s totalAllocated_/currentUsed_/peakUsed_/allocations_
// - getStats(), only available when EnableStats is true
// - peakUsed_ surviving a rollback that drops currentUsed_

#include <support/framework.h>

using namespace StackPro;

static void run_examples() {

    // EnableStats = true adds a Stats block to the stack, updated on
    // every allocate() and freeToMarker().
    setTitle("Enabling Stats");

    Stack<true> stack(1024);

    (void)stack.allocate(64);
    (void)stack.allocate(128);
    (void)stack.allocate(32);

    const auto& stats = stack.getStats();
    std::cout << "totalAllocated: " << stats.totalAllocated_ << "\n";
    std::cout << "currentUsed   : " << stats.currentUsed_ << "\n";
    std::cout << "peakUsed      : " << stats.peakUsed_ << "\n";
    std::cout << "allocations   : " << stats.allocations_ << "\n\n";

    // Rolling back drops currentUsed_ immediately, but peakUsed_ is a
    // high-water mark — it stays put, since it records the largest the
    // stack has ever been, not what's live right now.
    setTitle("Peak Survives a Rollback");

    Stack<true>::Marker before_growth = stack.getMarker();
    (void)stack.allocate(512); // pushes currentUsed_ and peakUsed_ up together

    std::cout << "currentUsed before rollback: " << stats.currentUsed_ << "\n";
    std::cout << "peakUsed before rollback   : " << stats.peakUsed_ << "\n\n";

    stack.freeToMarker(before_growth);

    std::cout << "currentUsed after rollback: " << stats.currentUsed_ << "\n";
    std::cout << "peakUsed after rollback   : " << stats.peakUsed_ << "\n\n";

    // totalAllocated_ and allocations_ are lifetime counters — they only
    // grow, even across rollbacks and reuse.
    setTitle("Lifetime Counters Keep Growing");

    (void)stack.allocate(16);
    std::cout << "totalAllocated after reuse: " << stats.totalAllocated_ << "\n";
    std::cout << "allocations after reuse   : " << stats.allocations_ << "\n\n";

    // reset() is the only thing that clears stats back to zero — it's a
    // full return to the stack's initial state, not just a rollback.
    setTitle("Reset Clears Everything");

    stack.reset();

    std::cout << "totalAllocated after reset(): " << stats.totalAllocated_ << "\n";
    std::cout << "peakUsed after reset()      : " << stats.peakUsed_ << "\n";
}

REGISTER_EXAMPLE_SUITE();
