// Stack alignment test suite.
//
// Coverage:
// - Default allocate() alignment matches alignof(std::max_align_t)
// - A custom construction alignment is honored by allocate()
// - A stricter per-call alignment consumes padding bytes as needed

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>

using namespace StackPro;

// Verifies allocate() defaults to alignof(std::max_align_t) when no
// alignment is requested.
TEST(Alignment, DefaultAllocateAlignment) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(8);

    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % alignof(std::max_align_t), 0u);
}

// Verifies a stricter construction-time alignment is honored by
// subsequent allocations that request it.
TEST(Alignment, CustomConstructionAlignmentHonored) {
    Stack<> stack(128, 64);
    std::byte* p = stack.allocate(8, 64);

    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % 64, 0u);
}

// Verifies a misaligned offset is padded forward to satisfy a stricter
// per-call alignment request, consuming more than just the requested size.
TEST(Alignment, PaddingConsumesCapacity) {
    Stack<> stack(64);
    (void)stack.allocate(1, 1);
    const std::size_t usedAfterSmall = stack.used();

    (void)stack.allocate(8, 8);

    EXPECT_GT(stack.used(), usedAfterSmall + 8);
}
