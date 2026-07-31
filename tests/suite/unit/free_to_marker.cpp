// Stack freeToMarker() test suite.
//
// Coverage:
// - Rewinds used() back to the marker's checkpoint
// - Freed space is reusable by a subsequent allocation
// - Freeing to the current marker is a no-op

#include <support/framework.h>

using namespace StackPro;

// Verifies freeToMarker() rewinds the offset to the checkpoint.
static void rewinds_offset_to_marker() {
    Stack<> stack(64);
    auto marker = stack.getMarker();
    (void)stack.allocate(16);

    stack.freeToMarker(marker);

    CHK(stack.used() == marker.get());
}

// Verifies space freed via a marker can be re-allocated, returning the
// same address.
static void freed_space_is_reusable() {
    Stack<> stack(16);
    auto marker = stack.getMarker();

    std::byte* first = stack.allocate(16);
    CHK(first != nullptr);

    stack.freeToMarker(marker);
    std::byte* second = stack.allocate(16);

    CHK(second != nullptr);
    CHK(first == second);
}

// Verifies freeing to a marker equal to the current offset changes nothing.
static void freeing_to_current_marker_is_noop() {
    Stack<> stack(64);
    (void)stack.allocate(8);
    auto marker = stack.getMarker();

    stack.freeToMarker(marker);

    CHK(stack.used() == marker.get());
}

// Executes all freeToMarker() test cases.
static void run_tests() {
    RUN(rewinds_offset_to_marker);
    RUN(freed_space_is_reusable);
    RUN(freeing_to_current_marker_is_noop);
}

REGISTER_TEST_SUITE();
