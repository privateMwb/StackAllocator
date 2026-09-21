// Nested StackScope integration suite.
//
// Coverage:
// - An inner scope's rollback leaves an outer scope's allocations intact
// - Deeply nested scopes unwind in LIFO order, each restoring its own marker
// - Sibling scopes at the same level each return to the same baseline offset

#include <support/framework.h>

using namespace StackPro;

// Verifies an inner scope only reclaims what it allocated, leaving the
// enclosing scope's allocations in place.
static void inner_leaves_outer_intact() {
    Stack<> stack(64);
    StackScope<> outer(stack);
    (void)stack.allocate(8);
    const std::size_t afterOuterAlloc = stack.used();

    {
        StackScope<> inner(stack);
        (void)stack.allocate(16);
    }

    CHK(stack.used() == afterOuterAlloc);
}

// Verifies three levels of nested scopes each roll back to their own
// marker in LIFO order as they unwind.
static void nested_unwind_lifo() {
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
                CHK(stack.used() == start + 24);
            }
            CHK(stack.used() == start + 16);
        }
        CHK(stack.used() == start + 8);
    }
    CHK(stack.used() == start);
}

// Verifies two sibling scopes, entered and exited one after the other,
// each return the stack to the same baseline offset.
static void siblings_same_offset() {
    Stack<> stack(64);
    const std::size_t start = stack.used();

    {
        StackScope<> first(stack);
        (void)stack.allocate(16);
    }
    CHK(stack.used() == start);

    {
        StackScope<> second(stack);
        (void)stack.allocate(8);
    }
    CHK(stack.used() == start);
}

// Executes all nested scope test cases.
static void run_tests() {
    RUN(inner_leaves_outer_intact);
    RUN(nested_unwind_lifo);
    RUN(siblings_same_offset);
}

REGISTER_TEST_SUITE();
