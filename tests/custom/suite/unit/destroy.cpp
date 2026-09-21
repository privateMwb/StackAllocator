// Stack destroy() test suite.
//
// Coverage:
// - Runs the object's destructor
// - Does not reclaim the object's storage
// - Is a no-op for a null pointer

#include <support/framework.h>

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
static void runs_destructor() {
    Stack<> stack(64);
    Tracked::destructed = 0;

    Tracked* p = stack.create<Tracked>();
    stack.destroy(p);

    CHK(Tracked::destructed == 1);
}

// Verifies destroy() does not roll back the bump offset; storage is only
// reclaimed via freeToMarker() or reset().
static void does_not_reclaim_storage() {
    Stack<> stack(64);
    Tracked* p = stack.create<Tracked>();
    const std::size_t usedBefore = stack.used();

    stack.destroy(p);

    CHK(stack.used() == usedBefore);
}

// Verifies destroying a null pointer is safe and runs no destructor.
static void null_pointer_is_noop() {
    Stack<> stack(64);
    Tracked::destructed = 0;

    stack.destroy(static_cast<Tracked*>(nullptr));

    CHK(Tracked::destructed == 0);
}

// Executes all destroy() test cases.
static void run_tests() {
    RUN(runs_destructor);
    RUN(does_not_reclaim_storage);
    RUN(null_pointer_is_noop);
}

REGISTER_TEST_SUITE();
