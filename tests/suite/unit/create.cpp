// Stack create() test suite.
//
// Coverage:
// - Forwards constructor arguments correctly
// - Returns nullptr without constructing when allocation fails
// - A throwing constructor propagates the exception
// - Storage is aligned to alignof(T)

#include <cstdint>
#include <stdexcept>
#include <support/framework.h>

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
static void forwards_constructor_arguments() {
    Stack<> stack(64);
    Point* p = stack.create<Point>(3, 4);

    CHK(p != nullptr);
    CHK(p->x == 3);
    CHK(p->y == 4);
}

// Verifies a failed allocation returns nullptr without running the
// constructor.
static void returns_nullptr_without_constructing() {
    Stack<> stack(1);
    Counted::count = 0;

    Counted* p = stack.create<Counted>();

    CHK(p == nullptr);
    CHK(Counted::count == 0);
}

// Verifies an exception thrown by the constructor propagates to the caller.
static void throwing_constructor_propagates() {
    Stack<> stack(64);
    CHK_THROWS(stack.create<Throws>(), std::runtime_error);
}

// Verifies the returned storage satisfies the type's alignment requirement.
static void storage_aligned_to_type() {
    Stack<> stack(128, 64);
    Aligned32* p = stack.create<Aligned32>();

    CHK(p != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(p) % alignof(Aligned32) == 0);
}

// Executes all create() test cases.
static void run_tests() {
    RUN(forwards_constructor_arguments);
    RUN(returns_nullptr_without_constructing);
    RUN(throwing_constructor_propagates);
    RUN(storage_aligned_to_type);
}

REGISTER_TEST_SUITE();
