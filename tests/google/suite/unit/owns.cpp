// Stack owns() test suite.
//
// Coverage:
// - A live allocation is owned
// - A foreign pointer is not owned
// - The one-past-the-end address is not owned
// - nullptr is not owned

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies a pointer just returned by allocate() is reported as owned.
TEST(Owns, LiveAllocationIsOwned) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(16);

    EXPECT_TRUE(stack.owns(p));
}

// Verifies a pointer unrelated to the buffer is not reported as owned.
TEST(Owns, ForeignPointerNotOwned) {
    Stack<> stack(64);
    int local = 0;

    EXPECT_FALSE(stack.owns(&local));
}

// Verifies the address one byte past the buffer's end is excluded.
TEST(Owns, OnePastEndNotOwned) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(64);

    ASSERT_NE(p, nullptr);
    EXPECT_FALSE(stack.owns(p + 64));
}

// Verifies nullptr is never reported as owned.
TEST(Owns, NullPointerNotOwned) {
    Stack<> stack(64);
    EXPECT_FALSE(stack.owns(nullptr));
}
