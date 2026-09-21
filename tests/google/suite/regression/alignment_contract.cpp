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

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <cstdint>
#include <gtest/gtest.h>
#include <utility>

using namespace StackPro;

// Verifies a per-call alignment equal to the construction alignment
// (the upper edge the precondition allows) is honored, not rejected.
TEST(AlignmentContract, ConstructAlignmentSucceeds) {
    Stack<> stack(128, 32);
    std::byte* p = stack.allocate(8, 32);

    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % 32, 0u);
}

// Verifies the common case — default alignment against default
// construction alignment — still works under the new precondition.
TEST(AlignmentContract, DefaultAlignmentBound) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(8);

    EXPECT_NE(p, nullptr);
}

// Verifies a moved-from stack's next default-alignment allocate() call
// fails cleanly via the capacity check instead of aborting on the
// alignment precondition.
TEST(AlignmentContract, MovedFromDefaultAlloc) {
    Stack<> source(64);
    Stack<> dest(std::move(source));

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    std::byte* p = source.allocate(8);
    EXPECT_EQ(p, nullptr);
}
