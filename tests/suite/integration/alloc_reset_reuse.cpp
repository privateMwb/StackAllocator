// Stack alloc/reset/reuse integration suite.
//
// Coverage:
// - Filling the buffer to exact capacity succeeds; the next request fails
// - reset() reclaims the full buffer, allowing it to be filled again
// - Addresses handed out after reset() match those from before it

#include <support/framework.h>

using namespace StackPro;

// Verifies a request matching the full remaining capacity succeeds, and
// anything past that fails.
static void fill_to_capacity_then_overflow_fails() {
    Stack<> stack(32);
    std::byte* p = stack.allocate(32);

    CHK(p != nullptr);
    CHK(stack.remaining() == 0);

    std::byte* overflow = stack.allocate(1);
    CHK(overflow == nullptr);
}

// Verifies reset() gives back the entire buffer, allowing it to be
// filled to capacity a second time.
static void reset_reclaims_full_capacity() {
    Stack<> stack(32);
    (void)stack.allocate(32);

    stack.reset();
    CHK(stack.remaining() == 32);

    std::byte* p = stack.allocate(32);
    CHK(p != nullptr);
}

// Verifies an allocation made after reset() lands at the same address
// as one made before it, since the buffer isn't reallocated.
static void reused_addresses_match_after_reset() {
    Stack<> stack(32);
    std::byte* first = stack.allocate(16);

    stack.reset();
    std::byte* second = stack.allocate(16);

    CHK(first == second);
}

// Executes all alloc/reset/reuse test cases.
static void run_tests() {
    RUN(fill_to_capacity_then_overflow_fails);
    RUN(reset_reclaims_full_capacity);
    RUN(reused_addresses_match_after_reset);
}

REGISTER_TEST_SUITE();
