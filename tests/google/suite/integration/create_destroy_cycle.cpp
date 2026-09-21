// Stack create/destroy cycle integration suite.
//
// Coverage:
// - destroy() runs the destructor exactly once and doesn't get invoked
//   again by a later reset()
// - create() after destroy() (without reclaiming storage) allocates a
//   fresh slot rather than reusing the destroyed one
// - freeToMarker() past a destroyed object allows a later create() to
//   reuse that same memory

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>

using namespace StackPro;

namespace {

struct Tracked {
    static int destructed;
    int value = 0;
    ~Tracked() {
        ++destructed;
    }
};
int Tracked::destructed = 0;

} // namespace

// Verifies destroy() runs the destructor exactly once, and that a
// subsequent reset() does not invoke it again.
TEST(CreateDestroyCycle, DestructorRunsOnce) {
    Tracked::destructed = 0;
    Stack<> stack(64);

    Tracked* p = stack.create<Tracked>();
    stack.destroy(p);
    EXPECT_EQ(Tracked::destructed, 1);

    stack.reset();
    EXPECT_EQ(Tracked::destructed, 1);
}

// Verifies destroy() alone doesn't reclaim storage, so a later create()
// allocates further along rather than reusing the destroyed slot.
TEST(CreateDestroyCycle, DestroyCreateNoReuse) {
    Stack<> stack(64);

    Tracked* first = stack.create<Tracked>();
    stack.destroy(first);
    Tracked* second = stack.create<Tracked>();

    EXPECT_NE(first, second);
}

// Verifies rolling back to a marker taken before a destroyed object
// frees its storage for reuse by a later create().
TEST(CreateDestroyCycle, FreeMarkerAfterDestroy) {
    Stack<> stack(64);
    auto marker = stack.getMarker();

    Tracked* first = stack.create<Tracked>();
    stack.destroy(first);
    stack.freeToMarker(marker);

    Tracked* second = stack.create<Tracked>();
    EXPECT_EQ(first, second);
}
