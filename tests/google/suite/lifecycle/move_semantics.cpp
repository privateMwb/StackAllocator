// Stack move semantics test suite.
//
// Coverage:
// - Move construction transfers capacity/usage state and empties the source
// - Move assignment releases the destination's old buffer and adopts
//   the source's state, emptying the source
// - A moved-from stack is safe to keep using (allocate() just fails)
// - Self-move-assignment is a no-op that leaves state untouched

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>
#include <utility>

using namespace StackPro;

// Verifies move construction transfers state and leaves the source empty.
TEST(MoveSemantics, MoveConstructionTransfersState) {
    Stack<> source(64);
    (void)source.allocate(16);

    Stack<> dest(std::move(source));

    EXPECT_EQ(dest.capacity(), 64u);
    EXPECT_EQ(dest.used(), 16u);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_EQ(source.capacity(), 0u);
    EXPECT_EQ(source.used(), 0u);
}

// Verifies move assignment releases the destination's prior buffer and
// adopts the source's state, leaving the source empty.
TEST(MoveSemantics, MoveAssignmentAdoptsSourceState) {
    Stack<> dest(32);
    (void)dest.allocate(8);

    Stack<> source(64);
    (void)source.allocate(16);

    dest = std::move(source);

    EXPECT_EQ(dest.capacity(), 64u);
    EXPECT_EQ(dest.used(), 16u);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_EQ(source.capacity(), 0u);
    EXPECT_EQ(source.used(), 0u);
}

// Verifies a moved-from stack is safe to keep calling: allocate() just
// fails cleanly since its capacity is now zero.
TEST(MoveSemantics, MovedFromStackIsSafeToUse) {
    Stack<> source(64);
    Stack<> dest(std::move(source));

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    std::byte* p = source.allocate(1);

    EXPECT_EQ(p, nullptr);
    EXPECT_EQ(source.used(), 0u);
    EXPECT_EQ(source.remaining(), 0u);
}

// Verifies self-move-assignment leaves the stack's state untouched.
TEST(MoveSemantics, SelfMoveAssignmentIsNoop) {
    Stack<> stack(64);
    (void)stack.allocate(16);

    Stack<>* self = &stack;
    stack = std::move(*self);

    EXPECT_EQ(stack.capacity(), 64u);
    EXPECT_EQ(stack.used(), 16u);
}
