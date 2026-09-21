// Stack destruction test suite.
//
// Coverage:
// - Destroying a moved-from stack is a safe no-op, not a double free
// - Repeated construct/destroy cycles run cleanly (catches leaks and
//   double frees under a sanitizer or valgrind run)
// - Destroying a stack that still holds live, un-destroyed objects is
//   safe — the buffer is simply released; object destructors are the
//   caller's responsibility

#include <support/framework.h>

using namespace StackPro;

// Verifies a moved-from stack's destructor doesn't double-free the
// buffer that was already transferred to the move destination.
static void destroying_moved_from_stack_is_safe() {
    Stack<> source(32);
    Stack<> destination(std::move(source));

    // source's destructor runs when this function returns, with
    // memory_ already nulled out by the move. Must not crash.
    CHK(destination.capacity() == 32);
}

// Verifies many construct/destroy cycles in a row don't crash or leak.
static void repeated_construct_destroy_is_stable() {
    for (int i = 0; i < 1000; ++i) {
        Stack<> stack(64);
        (void)stack.allocate(32);
    }
    CHK(true);
}

// Verifies a stack going out of scope with live, un-destroyed objects
// simply releases its buffer without crashing.
static void destructor_is_safe_with_live_allocations() {
    {
        Stack<> stack(64);
        (void)stack.create<int>(42);
        (void)stack.allocate(16);
        // stack destructs here; it does not run ~int() for the
        // created object, only releases the underlying buffer.
    }
    CHK(true);
}

// Executes all destruction test cases.
static void run_tests() {
    RUN(destroying_moved_from_stack_is_safe);
    RUN(repeated_construct_destroy_is_stable);
    RUN(destructor_is_safe_with_live_allocations);
}

REGISTER_TEST_SUITE();
