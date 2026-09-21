// Stack destroy() test suite.
//
// Coverage:
// - Runs the object's destructor
// - Does not reclaim the object's storage
// - Is a no-op for a null pointer

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

namespace {

struct Tracked {
    static int destructed;
    ~Tracked() {
        ++destructed;
    }
};
int Tracked::destructed = 0;

} // namespace

// Verifies destroy() invokes the object's destructor.
TEST(Destroy, RunsDestructor) {
    Stack<> stack(64);
    Tracked::destructed = 0;

    Tracked* p = stack.create<Tracked>();
    stack.destroy(p);

    EXPECT_EQ(Tracked::destructed, 1);
}

// Verifies destroy() does not roll back the bump offset; storage is only
// reclaimed via freeToMarker() or reset().
TEST(Destroy, DoesNotReclaimStorage) {
    Stack<> stack(64);
    Tracked* p = stack.create<Tracked>();
    const std::size_t usedBefore = stack.used();

    stack.destroy(p);

    EXPECT_EQ(stack.used(), usedBefore);
}

// Verifies destroying a null pointer is safe and runs no destructor.
TEST(Destroy, NullPointerIsNoop) {
    Stack<> stack(64);
    Tracked::destructed = 0;

    stack.destroy(static_cast<Tracked*>(nullptr));

    EXPECT_EQ(Tracked::destructed, 0);
}
