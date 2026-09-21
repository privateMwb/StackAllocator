// Stack allocate() test suite.
//
// Coverage:
// - Returns a non-null pointer when capacity allows
// - Returns nullptr when the request exceeds remaining capacity
// - Successive allocations advance the offset
// - Returned pointer honors the requested alignment
// - An oversized request near SIZE_MAX fails cleanly instead of
//   overflowing the internal bounds check

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <cstdint>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies a request that fits in the buffer succeeds.
TEST(Allocate, ReturnsPointerWithinCapacity) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(16);
    EXPECT_NE(p, nullptr);
}

// Verifies a request larger than the remaining capacity fails.
TEST(Allocate, ReturnsNullptrWhenOutOfSpace) {
    Stack<> stack(8);
    std::byte* p = stack.allocate(16);
    EXPECT_EQ(p, nullptr);
}

// Verifies the bump-pointer offset moves forward after each allocation.
TEST(Allocate, SuccessiveAllocationsAdvanceOffset) {
    Stack<> stack(64);
    const std::size_t before = stack.used();

    (void)stack.allocate(8);
    const std::size_t afterFirst = stack.used();
    EXPECT_GT(afterFirst, before);

    (void)stack.allocate(8);
    EXPECT_GT(stack.used(), afterFirst);
}

// Verifies the returned pointer is aligned to the requested alignment.
TEST(Allocate, RespectsRequestedAlignment) {
    Stack<> stack(128, 64);

    // Force a misaligned offset first, then request a 16-byte-aligned block.
    (void)stack.allocate(3, 1);
    std::byte* p = stack.allocate(16, 16);

    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % 16, 0u);
}

// Verifies a request whose size would overflow the internal bounds
// check (offset + size) fails cleanly instead of wrapping around and
// succeeding with too little real capacity.
TEST(Allocate, OversizedRequestDoesNotOverflow) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(SIZE_MAX - 4);
    EXPECT_EQ(p, nullptr);
}
