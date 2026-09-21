// StackScope rollback integration suite.
//
// Coverage:
// - Allocations made inside a scope are reclaimed on normal exit
// - Allocations made inside a scope are reclaimed when the scope
//   unwinds due to an exception
// - Allocations made before the scope was entered survive it

#include <stdexcept>
#include <support/framework.h>

using namespace StackPro;

// Verifies allocations inside a scope are rolled back when it exits normally.
static void normal_exit_reclaims() {
    Stack<> stack(64);
    (void)stack.allocate(8);
    const std::size_t baseline = stack.used();

    {
        StackScope<> scope(stack);
        (void)stack.allocate(16);
        (void)stack.allocate(8);
    }

    CHK(stack.used() == baseline);
}

// Verifies allocations inside a scope are rolled back even when the
// scope is left via stack unwinding from an exception.
static void exception_unwind_reclaims() {
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

    CHK(stack.used() == baseline);
}

// Verifies allocations made before a scope was entered are untouched
// once the scope ends.
static void before_scope_survives() {
    Stack<> stack(64);
    std::byte* pre = stack.allocate(8);

    {
        StackScope<> scope(stack);
        (void)stack.allocate(16);
    }

    CHK(stack.owns(pre));
    CHK(stack.used() == 8);
}

// Executes all scope rollback test cases.
static void run_tests() {
    RUN(normal_exit_reclaims);
    RUN(exception_unwind_reclaims);
    RUN(before_scope_survives);
}

REGISTER_TEST_SUITE();
