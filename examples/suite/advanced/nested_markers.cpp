// Nested markers.
//
// Demonstrates:
// - Stacking several markers, LIFO-style
// - Rolling back an inner marker without disturbing outer ones
// - No fixed nesting-depth limit — a marker is just a saved offset

#include <support/framework.h>

using namespace StackPro;

static void run_examples() {

    // Each getMarker() call is independent — nothing stops you from
    // taking more than one, at any depth.
    setTitle("Taking Several Markers");

    Stack<> stack(1024);

    (void)stack.allocate(64);
    Stack<>::Marker outer = stack.getMarker();
    std::cout << "outer marker at: " << outer.get() << "\n";

    (void)stack.allocate(64);
    Stack<>::Marker middle = stack.getMarker();
    std::cout << "middle marker at: " << middle.get() << "\n";

    (void)stack.allocate(64);
    Stack<>::Marker inner = stack.getMarker();
    std::cout << "inner marker at: " << inner.get() << "\n\n";

    // Rolling back to the innermost marker only undoes what happened
    // after it — outer and middle are untouched.
    setTitle("Rolling Back the Innermost");

    (void)stack.allocate(128);
    std::cout << "used before rollback: " << stack.used() << "\n";

    stack.freeToMarker(inner);
    std::cout << "used after rolling back to inner: " << stack.used() << "\n\n";

    // Rolling back further, to middle, also silently discards inner —
    // there's nothing to "close" first, since markers carry no state of
    // their own beyond the offset.
    setTitle("Skipping Straight to an Outer Marker");

    stack.freeToMarker(middle);
    std::cout << "used after rolling back to middle: " << stack.used() << "\n";

    stack.freeToMarker(outer);
    std::cout << "used after rolling back to outer: " << stack.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
