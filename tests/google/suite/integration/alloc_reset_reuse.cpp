// Stack alloc/reset/reuse integration suite.
//
// Coverage:
// - Filling the buffer to exact capacity succeeds; the next request fails
// - reset() reclaims the full buffer, allowing it to be filled again
// - Addresses handed out after reset() match those from before it

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies a request matching the full remaining capacity succeeds, and
// anything past that fails.
TEST(AllocResetReuse, FillThenOverflowFails) {
    Stack<> stack(32);
    std::byte* p = stack.allocate(32);

    EXPECT_NE(p, nullptr);
    EXPECT_EQ(stack.remaining(), 0u);

    std::byte* overflow = stack.allocate(1);
    EXPECT_EQ(overflow, nullptr);
}

// Verifies reset() gives back the entire buffer, allowing it to be
// filled to capacity a second time.
TEST(AllocResetReuse, ResetReclaimsCapacity) {
    Stack<> stack(32);
    (void)stack.allocate(32);

    stack.reset();
    EXPECT_EQ(stack.remaining(), 32u);

    std::byte* p = stack.allocate(32);
    EXPECT_NE(p, nullptr);
}

// Verifies an allocation made after reset() lands at the same address
// as one made before it, since the buffer isn't reallocated.
TEST(AllocResetReuse, ReusedAddrAfterReset) {
    Stack<> stack(32);
    std::byte* first = stack.allocate(16);

    stack.reset();
    std::byte* second = stack.allocate(16);

    EXPECT_EQ(first, second);
}
