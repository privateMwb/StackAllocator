// Stack create() test suite.
//
// Coverage:
// - Forwards constructor arguments correctly
// - Returns nullptr without constructing when allocation fails
// - A throwing constructor propagates the exception
// - Storage is aligned to alignof(T)

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <cstdint>
#include <gtest/gtest.h>
#include <stdexcept>

using namespace StackPro;

namespace {

struct Point {
    int x;
    int y;
    Point(int x_, int y_) : x(x_), y(y_) {}
};

struct Counted {
    static int count;
    int value;
    Counted() : value(0) {
        ++count;
    }
};
int Counted::count = 0;

struct Throws {
    Throws() {
        throw std::runtime_error("boom");
    }
};

struct alignas(32) Aligned32 {
    char data[32];
};

} // namespace

// Verifies constructor arguments reach the constructed object unchanged.
TEST(Create, ForwardsConstructorArguments) {
    Stack<> stack(64);
    Point* p = stack.create<Point>(3, 4);

    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 3);
    EXPECT_EQ(p->y, 4);
}

// Verifies a failed allocation returns nullptr without running the
// constructor.
TEST(Create, ReturnsNullptrWithoutConstructing) {
    Stack<> stack(1);
    Counted::count = 0;

    Counted* p = stack.create<Counted>();

    EXPECT_EQ(p, nullptr);
    EXPECT_EQ(Counted::count, 0);
}

// Verifies an exception thrown by the constructor propagates to the caller.
TEST(Create, ThrowingConstructorPropagates) {
    Stack<> stack(64);
    EXPECT_THROW(stack.create<Throws>(), std::runtime_error);
}

// Verifies the returned storage satisfies the type's alignment requirement.
TEST(Create, StorageAlignedToType) {
    Stack<> stack(128, 64);
    Aligned32* p = stack.create<Aligned32>();

    EXPECT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % alignof(Aligned32), 0u);
}
