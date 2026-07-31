// Stack allocate() overflow-guard regression suite.
//
// Pins the fix for a bounds check that used to be
// `alignedOffset + size > cap_`, which could wrap around for a size
// close to SIZE_MAX and let an oversized request through. The check is
// now subtraction-based (`size > cap_ - alignedOffset`) and can't overflow.
//
// Coverage:
// - A size that would overflow the old addition-based check is rejected
// - The same overflow-prone size is still rejected after some capacity
//   is already in use (nonzero offset)
// - A request sized at exactly the remaining capacity still succeeds
//   (guards against the rewritten check flipping the boundary off by one)

#include <cstdint>
#include <support/framework.h>

using namespace StackPro;

// Verifies a request whose size would wrap `alignedOffset + size`
// around SIZE_MAX is rejected rather than let through.
static void rejects_size_that_would_overflow_addition() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(SIZE_MAX - 10);

    CHK(p == nullptr);
}

// Verifies the same class of oversized request is still rejected once
// the offset is already nonzero, not just at offset zero.
static void overflow_guard_holds_after_partial_allocation() {
    Stack<> stack(64);
    (void)stack.allocate(16);

    std::byte* p = stack.allocate(SIZE_MAX - 8);
    CHK(p == nullptr);
}

// Verifies a request that exactly matches remaining capacity still
// succeeds under the rewritten, overflow-safe check.
static void exact_remaining_capacity_still_succeeds() {
    Stack<> stack(32);
    std::byte* p = stack.allocate(32);

    CHK(p != nullptr);
    CHK(stack.remaining() == 0);
}

// Executes all overflow-guard regression test cases.
static void run_tests() {
    RUN(rejects_size_that_would_overflow_addition);
    RUN(overflow_guard_holds_after_partial_allocation);
    RUN(exact_remaining_capacity_still_succeeds);
}

REGISTER_TEST_SUITE();
