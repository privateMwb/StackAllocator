// Stack create/destroy cycle integration suite.
//
// Coverage:
// - destroy() runs the destructor exactly once and doesn't get invoked
//   again by a later reset()
// - create() after destroy() (without reclaiming storage) allocates a
//   fresh slot rather than reusing the destroyed one
// - freeToMarker() past a destroyed object allows a later create() to
//   reuse that same memory

#include <support/framework.h>

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
static void destructor_runs_once() {
    Tracked::destructed = 0;
    Stack<> stack(64);

    Tracked* p = stack.create<Tracked>();
    stack.destroy(p);
    CHK(Tracked::destructed == 1);

    stack.reset();
    CHK(Tracked::destructed == 1);
}

// Verifies destroy() alone doesn't reclaim storage, so a later create()
// allocates further along rather than reusing the destroyed slot.
static void destroy_create_no_reuse() {
    Stack<> stack(64);

    Tracked* first = stack.create<Tracked>();
    stack.destroy(first);
    Tracked* second = stack.create<Tracked>();

    CHK(first != second);
}

// Verifies rolling back to a marker taken before a destroyed object
// frees its storage for reuse by a later create().
static void free_marker_after_destroy() {
    Stack<> stack(64);
    auto marker = stack.getMarker();

    Tracked* first = stack.create<Tracked>();
    stack.destroy(first);
    stack.freeToMarker(marker);

    Tracked* second = stack.create<Tracked>();
    CHK(first == second);
}

// Executes all create/destroy cycle test cases.
static void run_tests() {
    RUN(destructor_runs_once);
    RUN(destroy_create_no_reuse);
    RUN(free_marker_after_destroy);
}

REGISTER_TEST_SUITE();
