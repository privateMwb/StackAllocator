// Stack freeToMarker() test suite.
//
// Coverage:
// - Rewinds used() back to the marker's checkpoint
// - Freed space is reusable by a subsequent allocation
// - Freeing to the current marker is a no-op

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies freeToMarker() rewinds the offset to the checkpoint.
TEST(FreeToMarker, RewindsOffsetToMarker) {
    Stack<> stack(64);
    auto marker = stack.getMarker();
    (void)stack.allocate(16);

    stack.freeToMarker(marker);

    EXPECT_EQ(stack.used(), marker.get());
}

// Verifies space freed via a marker can be re-allocated, returning the
// same address.
TEST(FreeToMarker, FreedSpaceIsReusable) {
    Stack<> stack(16);
    auto marker = stack.getMarker();

    std::byte* first = stack.allocate(16);
    EXPECT_NE(first, nullptr);

    stack.freeToMarker(marker);
    std::byte* second = stack.allocate(16);

    EXPECT_NE(second, nullptr);
    EXPECT_EQ(first, second);
}

// Verifies freeing to a marker equal to the current offset changes nothing.
TEST(FreeToMarker, FreeingToCurrentMarkerIsNoop) {
    Stack<> stack(64);
    (void)stack.allocate(8);
    auto marker = stack.getMarker();

    stack.freeToMarker(marker);

    EXPECT_EQ(stack.used(), marker.get());
}
