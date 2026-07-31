// Stack alignment contract regression suite.
//
// Pins two fixes:
// 1. allocate() now asserts `request_alignment <= construction
//    alignment` in debug builds, where it previously had no such check.
// 2. That precondition then had a false trip of its own: move
//    construction/assignment used to reset `alignShift_` to 0, so a
//    moved-from stack's *default*-alignment allocate() call (16 bytes,
//    typically) exceeded its now-1-byte "construction alignment" and
//    aborted, instead of just failing the capacity check like any
//    other call on an empty stack. alignShift_ is no longer reset on
//    move for exactly this reason.
//
// Coverage:
// - Requesting exactly the construction alignment succeeds
// - The default alignment succeeds against the default construction
//   alignment (the common case the precondition must not break)
// - A moved-from stack's default-alignment allocate() call fails via
//   the capacity check, not by aborting on the precondition

#include <support/framework.h>

using namespace StackPro;

// Verifies a per-call alignment equal to the construction alignment
// (the upper edge the precondition allows) is honored, not rejected.
static void request_at_construction_alignment_succeeds() {
    Stack<> stack(128, 32);
    std::byte* p = stack.allocate(8, 32);

    CHK(p != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(p) % 32 == 0);
}

// Verifies the common case — default alignment against default
// construction alignment — still works under the new precondition.
static void default_alignment_within_default_bound() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(8);

    CHK(p != nullptr);
}

// Verifies a moved-from stack's next default-alignment allocate() call
// fails cleanly via the capacity check instead of aborting on the
// alignment precondition.
static void moved_from_stack_default_allocate_does_not_abort() {
    Stack<> source(64);
    Stack<> dest(std::move(source));

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    std::byte* p = source.allocate(8);
    CHK(p == nullptr);
}

// Executes all alignment contract regression test cases.
static void run_tests() {
    RUN(request_at_construction_alignment_succeeds);
    RUN(default_alignment_within_default_bound);
    RUN(moved_from_stack_default_allocate_does_not_abort);
}

REGISTER_TEST_SUITE();
