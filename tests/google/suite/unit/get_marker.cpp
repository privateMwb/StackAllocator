// Stack getMarker() test suite.
//
// Coverage:
// - A marker taken before any allocation reads as zero
// - A marker reflects the current used() offset
// - A marker taken later reads higher than one taken earlier

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies a marker taken on a fresh stack is zero.
TEST(GetMarker, MarkerAtStartIsZero) {
    Stack<> stack(64);
    auto marker = stack.getMarker();

    EXPECT_EQ(marker.get(), 0u);
}

// Verifies a marker matches used() at the moment it is taken.
TEST(GetMarker, MarkerReflectsCurrentOffset) {
    Stack<> stack(64);
    (void)stack.allocate(10);

    auto marker = stack.getMarker();

    EXPECT_EQ(marker.get(), stack.used());
}

// Verifies a marker taken after further allocation is greater than one
// taken earlier.
TEST(GetMarker, LaterMarkerIsGreater) {
    Stack<> stack(64);
    auto first = stack.getMarker();

    (void)stack.allocate(8);
    auto second = stack.getMarker();

    EXPECT_GT(second.get(), first.get());
}
