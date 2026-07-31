// Move semantics.
//
// Demonstrates:
// - Move construction and move assignment transferring ownership
// - What the moved-from stack looks like afterward
// - Which operations remain safe to call on a moved-from stack

#include <support/framework.h>

using namespace StackPro;

static void run_examples() {

    // Moving a stack transfers ownership of its buffer — the source is
    // left empty (capacity 0) rather than destroyed.
    setTitle("Move Construction");

    Stack<> source(1024);
    (void)source.allocate(64);
    std::cout << "source used before move: " << source.used() << "\n";

    Stack<> dest(std::move(source));

    std::cout << "dest capacity  : " << dest.capacity() << "\n";
    std::cout << "dest used      : " << dest.used() << "\n";
    std::cout << "source capacity: " << source.capacity() << "\n";
    std::cout << "source used    : " << source.used() << "\n\n";

    // Move assignment releases the destination's own buffer first, then
    // takes ownership of the source's.
    setTitle("Move Assignment");

    Stack<> other(256);
    (void)other.allocate(32);
    std::cout << "other capacity before: " << other.capacity() << "\n";

    other = std::move(dest);

    std::cout << "other capacity after: " << other.capacity() << "\n";
    std::cout << "other used after    : " << other.used() << "\n\n";

    // A moved-from stack has zero capacity, so every query on it reports
    // empty rather than trapping — allocate() simply fails like it would
    // on any exhausted stack.
    setTitle("Moved-From Is Safe, Not Usable");

    std::cout << "dest capacity : " << dest.capacity() << "\n";
    std::cout << "dest remaining: " << dest.remaining() << "\n";

    std::byte* block = dest.allocate(16);
    std::cout << "allocate() on moved-from succeeded: " << (block != nullptr) << "\n";

    Stack<>::Marker marker = dest.getMarker();
    std::cout << "getMarker() on moved-from: " << marker.get() << "\n";

    dest.freeToMarker(marker);
    dest.reset();
    std::cout << "reset()/freeToMarker() on moved-from: no-op, no crash\n";
}

REGISTER_EXAMPLE_SUITE();
