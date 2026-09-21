// Stack destruction test suite.
//
// Coverage:
// - Destroying a moved-from stack is a safe no-op, not a double free
// - Repeated construct/destroy cycles run cleanly (catches leaks and
//   double frees under a sanitizer or valgrind run)
// - Destroying a stack that still holds live, un-destroyed objects is
//   safe — the buffer is simply released; object destructors are the
//   caller's responsibility

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>
#include <utility>

using namespace StackPro;

// Verifies a moved-from stack's destructor doesn't double-free the
// buffer that was already transferred to the move destination.
TEST(Destruction, DestroyingMovedFromStackIsSafe) {
    Stack<> source(32);
    Stack<> destination(std::move(source));

    // source's destructor runs when this function returns, with
    // memory_ already nulled out by the move. Must not crash.
    EXPECT_EQ(destination.capacity(), 32u);
}

// Verifies many construct/destroy cycles in a row don't crash or leak.
TEST(Destruction, RepeatedConstructDestroyIsStable) {
    for (int i = 0; i < 1000; ++i) {
        Stack<> stack(64);
        (void)stack.allocate(32);
    }
    SUCCEED();
}

// Verifies a stack going out of scope with live, un-destroyed objects
// simply releases its buffer without crashing.
TEST(Destruction, DestructorIsSafeWithLiveAllocations) {
    {
        Stack<> stack(64);
        (void)stack.create<int>(42);
        (void)stack.allocate(16);
        // stack destructs here; it does not run ~int() for the
        // created object, only releases the underlying buffer.
    }
    SUCCEED();
}
