// Nested StackScope integration suite.
//
// Coverage:
// - An inner scope's rollback leaves an outer scope's allocations intact
// - Deeply nested scopes unwind in LIFO order, each restoring its own marker
// - Sibling scopes at the same level each return to the same baseline offset

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies an inner scope only reclaims what it allocated, leaving the
// enclosing scope's allocations in place.
TEST(NestedScopes, InnerLeavesOuterIntact) {
    Stack<> stack(64);
    StackScope<> outer(stack);
    (void)stack.allocate(8);
    const std::size_t afterOuterAlloc = stack.used();

    {
        StackScope<> inner(stack);
        (void)stack.allocate(16);
    }

    EXPECT_EQ(stack.used(), afterOuterAlloc);
}

// Verifies three levels of nested scopes each roll back to their own
// marker in LIFO order as they unwind.
TEST(NestedScopes, NestedUnwindLifo) {
    Stack<> stack(64);
    const std::size_t start = stack.used();

    {
        StackScope<> level1(stack);
        (void)stack.allocate(8, 1);
        {
            StackScope<> level2(stack);
            (void)stack.allocate(8, 1);
            {
                StackScope<> level3(stack);
                (void)stack.allocate(8, 1);
                EXPECT_EQ(stack.used(), start + 24);
            }
            EXPECT_EQ(stack.used(), start + 16);
        }
        EXPECT_EQ(stack.used(), start + 8);
    }
    EXPECT_EQ(stack.used(), start);
}

// Verifies two sibling scopes, entered and exited one after the other,
// each return the stack to the same baseline offset.
TEST(NestedScopes, SiblingsSameOffset) {
    Stack<> stack(64);
    const std::size_t start = stack.used();

    {
        StackScope<> first(stack);
        (void)stack.allocate(16);
    }
    EXPECT_EQ(stack.used(), start);

    {
        StackScope<> second(stack);
        (void)stack.allocate(8);
    }
    EXPECT_EQ(stack.used(), start);
}
