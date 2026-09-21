// Stack getMarker() test suite.
//
// Coverage:
// - A marker taken before any allocation reads as zero
// - A marker reflects the current used() offset
// - A marker taken later reads higher than one taken earlier

#include <support/framework.h>

using namespace StackPro;

// Verifies a marker taken on a fresh stack is zero.
static void marker_at_start_is_zero() {
    Stack<> stack(64);
    auto marker = stack.getMarker();

    CHK(marker.get() == 0);
}

// Verifies a marker matches used() at the moment it is taken.
static void marker_reflects_current_offset() {
    Stack<> stack(64);
    (void)stack.allocate(10);

    auto marker = stack.getMarker();

    CHK(marker.get() == stack.used());
}

// Verifies a marker taken after further allocation is greater than one
// taken earlier.
static void later_marker_is_greater() {
    Stack<> stack(64);
    auto first = stack.getMarker();

    (void)stack.allocate(8);
    auto second = stack.getMarker();

    CHK(second.get() > first.get());
}

// Executes all getMarker() test cases.
static void run_tests() {
    RUN(marker_at_start_is_zero);
    RUN(marker_reflects_current_offset);
    RUN(later_marker_is_greater);
}

REGISTER_TEST_SUITE();
