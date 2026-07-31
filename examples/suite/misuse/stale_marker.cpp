// Reusing a stale marker.
//
// Demonstrates:
// - freeToMarker() accepting a marker that's technically still valid
//   (not ahead of the current offset) but no longer means what the
//   caller thinks it means
// - How this silently discards more than intended, with no assertion
//   to catch it

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Entry {
    int id;
};

} // namespace

static void run_examples() {

    // freeToMarker()'s only contract is "not ahead of the current
    // offset" — it has no way to know a marker is logically outdated.
    setTitle("Two Markers, One Forgotten");

    Stack<> stack(1024);

    Stack<>::Marker checkpoint_a = stack.getMarker();
    (void)stack.create<Entry>(1);

    Stack<>::Marker checkpoint_b = stack.getMarker();
    (void)stack.create<Entry>(2);
    (void)stack.create<Entry>(3);

    std::cout << "used with entries 1-3: " << stack.used() << "\n\n";

    // The bug: the caller meant to roll back to checkpoint_b (undo just
    // entries 2 and 3) but accidentally reused checkpoint_a instead —
    // maybe because it was captured earlier and never re-fetched after
    // some refactor. Both are valid markers, so nothing fails loudly.
    setTitle("Rolling Back to the Wrong Marker");

    stack.freeToMarker(checkpoint_a); // meant: checkpoint_b

    std::cout << "used after wrong rollback: " << stack.used() << "\n";
    std::cout << "entry 1 was also discarded, not just 2 and 3\n\n";

    // The fix is discipline, not a runtime check: keep a marker's scope
    // as narrow and short-lived as possible, and re-fetch a fresh one
    // rather than holding onto an old one across unrelated work.
    setTitle("Avoiding It");

    std::cout << "prefer freeToMarker(stack.getMarker()) taken right\n";
    std::cout << "before the section being rolled back, not a marker\n";
    std::cout << "held from earlier in the function.\n";
}

REGISTER_EXAMPLE_SUITE();
