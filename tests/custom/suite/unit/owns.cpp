// Stack owns() test suite.
//
// Coverage:
// - A live allocation is owned
// - A foreign pointer is not owned
// - The one-past-the-end address is not owned
// - nullptr is not owned

#include <support/framework.h>

using namespace StackPro;

// Verifies a pointer just returned by allocate() is reported as owned.
static void live_allocation_is_owned() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(16);

    CHK(stack.owns(p));
}

// Verifies a pointer unrelated to the buffer is not reported as owned.
static void foreign_pointer_not_owned() {
    Stack<> stack(64);
    int local = 0;

    CHK(!stack.owns(&local));
}

// Verifies the address one byte past the buffer's end is excluded.
static void one_past_end_not_owned() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(64);

    CHK(p != nullptr);
    CHK(!stack.owns(p + 64));
}

// Verifies nullptr is never reported as owned.
static void null_pointer_not_owned() {
    Stack<> stack(64);
    CHK(!stack.owns(nullptr));
}

// Executes all owns() test cases.
static void run_tests() {
    RUN(live_allocation_is_owned);
    RUN(foreign_pointer_not_owned);
    RUN(one_past_end_not_owned);
    RUN(null_pointer_not_owned);
}

REGISTER_TEST_SUITE();
