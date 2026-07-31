// Basic Stack usage.
//
// Demonstrates:
// - Constructing a stack with a fixed buffer size
// - Raw allocation with allocate()
// - Construction in place with create<T>()
// - Destroying an object with destroy()
// - Capacity, used, and remaining checks
// - Resetting a stack

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Point {
    int x;
    int y;
};

} // namespace

static void run_examples() {

    // A stack is constructed with a fixed buffer size — the total number
    // of bytes it will ever be able to hand out.
    setTitle("Construction");

    Stack<> stack(1024);

    std::cout << "capacity : " << stack.capacity() << "\n";
    std::cout << "used     : " << stack.used() << "\n";
    std::cout << "remaining: " << stack.remaining() << "\n\n";

    // allocate() hands back raw, uninitialized bytes carved out of the
    // buffer, or nullptr if the request can't be satisfied.
    setTitle("Raw Allocation");

    std::byte* block = stack.allocate(64);
    std::cout << "allocate(64) succeeded: " << (block != nullptr) << "\n";
    std::cout << "used after allocate   : " << stack.used() << "\n\n";

    // create<T>() reserves storage sized and aligned for T and constructs
    // it in place, returning a ready-to-use pointer.
    setTitle("Construction In Place");

    Point* p = stack.create<Point>(3, 4);
    std::cout << "point: (" << p->x << ", " << p->y << ")\n\n";

    // destroy() runs the object's destructor. The underlying bytes stay
    // reserved — the stack has no concept of freeing a single allocation.
    setTitle("Destruction");

    stack.destroy(p);
    std::cout << "used after destroy() (unchanged): " << stack.used() << "\n\n";

    // used() and remaining() move together; capacity() never changes.
    setTitle("Capacity and Usage");

    std::cout << "capacity : " << stack.capacity() << "\n";
    std::cout << "used     : " << stack.used() << "\n";
    std::cout << "remaining: " << stack.remaining() << "\n\n";

    // reset() rewinds the whole stack to empty in O(1), making every byte
    // available for reuse. It does not touch or zero the buffer itself.
    setTitle("Reset");

    stack.reset();

    std::cout << "used after reset()     : " << stack.used() << "\n";
    std::cout << "remaining after reset(): " << stack.remaining() << "\n";
}

REGISTER_EXAMPLE_SUITE();
