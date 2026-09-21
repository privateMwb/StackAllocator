// StackScope rollback integration suite.
//
// Coverage:
// - Allocations made inside a scope are reclaimed on normal exit
// - Allocations made inside a scope are reclaimed when the scope
//   unwinds due to an exception
// - Allocations made before the scope was entered survive it

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>
#include <stdexcept>

using namespace StackPro;

// Verifies allocations inside a scope are rolled back when it exits normally.
TEST(ScopeRollback, NormalExitReclaims) {
    Stack<> stack(64);
    (void)stack.allocate(8);
    const std::size_t baseline = stack.used();

    {
        StackScope<> scope(stack);
        (void)stack.allocate(16);
        (void)stack.allocate(8);
    }

    EXPECT_EQ(stack.used(), baseline);
}

// Verifies allocations inside a scope are rolled back even when the
// scope is left via stack unwinding from an exception.
TEST(ScopeRollback, ExceptionUnwindReclaims) {
    Stack<> stack(64);
    (void)stack.allocate(8);
    const std::size_t baseline = stack.used();

    try {
        StackScope<> scope(stack);
        (void)stack.allocate(16);
        throw std::runtime_error("boom");
    } catch (const std::runtime_error&) {
        // Expected; scope's destructor should have already rolled back.
    }

    EXPECT_EQ(stack.used(), baseline);
}

// Verifies allocations made before a scope was entered are untouched
// once the scope ends.
TEST(ScopeRollback, BeforeScopeSurvives) {
    Stack<> stack(64);
    std::byte* pre = stack.allocate(8);

    {
        StackScope<> scope(stack);
        (void)stack.allocate(16);
    }

    EXPECT_TRUE(stack.owns(pre));
    EXPECT_EQ(stack.used(), 8u);
}
