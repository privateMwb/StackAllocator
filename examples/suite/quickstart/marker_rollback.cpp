// Checkpoint and rollback with Marker.
//
// Demonstrates:
// - Capturing a checkpoint with getMarker()
// - Allocating and constructing after the checkpoint
// - Rolling back in O(1) with freeToMarker()
// - used() before, during, and after the rollback

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Point {
    int x;
    int y;
};

} // namespace

static void run_examples() {

    // getMarker() captures the stack's current offset — a checkpoint that
    // freeToMarker() can later roll back to.
    setTitle("Taking a Marker");

    Stack<> stack(1024);

    (void)stack.create<Point>(1, 1);
    std::cout << "used before marker: " << stack.used() << "\n";

    Stack<>::Marker marker = stack.getMarker();
    std::cout << "marker taken at   : " << marker.get() << "\n\n";

    // Everything allocated after the marker is temporary — it only needs
    // to live until the next rollback.
    setTitle("Allocating After the Marker");

    (void)stack.create<Point>(2, 2);
    (void)stack.create<Point>(3, 3);
    std::cout << "used after 2 more points: " << stack.used() << "\n\n";

    // freeToMarker() rewinds the stack to the captured offset in O(1),
    // reclaiming everything allocated since — regardless of how many
    // allocations that was.
    setTitle("Rolling Back");

    stack.freeToMarker(marker);

    std::cout << "used after freeToMarker(): " << stack.used() << "\n";
    std::cout << "matches marker offset     : " << (stack.used() == marker.get()) << "\n";
}

REGISTER_EXAMPLE_SUITE();
