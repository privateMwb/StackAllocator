// Stack constructor test suite.
//
// Coverage:
// - Basic construction
// - Default alignment
// - Initial state
// - Custom alignment
// - Large capacity

#include <common/framework.h>

using namespace AllocatorPro;

// Verifies that a stack initializes with the specified capacity.
static void basic_construction() {
    Stack s{1024};
    CHK(s.capacity() == 1024);
}

// Verifies that the default alignment produces a valid initial state.
static void default_alignment() {
    Stack s{1024};
    CHK(s.used()      == 0);
    CHK(s.remaining() == 1024);
}

// Verifies that a newly constructed stack contains no allocated memory.
static void initial_state() {
    Stack s{1024};
    CHK(s.used()      == 0);
    CHK(s.remaining() == s.capacity());
}

// Verifies that a custom power-of-two alignment is accepted.
static void custom_alignment() {
    Stack s{1024, 16};
    CHK(s.capacity()  == 1024);
    CHK(s.used()      == 0);
    CHK(s.remaining() == 1024);
}

// Verifies that a stack supports large allocation capacities.
static void large_capacity() {
    Stack s{1024 * 1024};
    CHK(s.capacity()  == 1024 * 1024);
    CHK(s.used()      == 0);
    CHK(s.remaining() == 1024 * 1024);
}

// Executes all constructor test cases.
static void run_tests() {
    RUN(basic_construction);
    RUN(default_alignment);
    RUN(initial_state);
    RUN(custom_alignment);
    RUN(large_capacity);
}

REGISTER_TEST_SUITE();