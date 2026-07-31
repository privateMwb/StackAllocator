// Object lifetime with Stack.
//
// Demonstrates:
// - Several create<T>() calls in sequence
// - Construction and destruction order
// - Why destroy() doesn't shrink used()
// - Reclaiming space only happens via freeToMarker() or reset()

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Widget {
    int id;

    explicit Widget(int id) : id{id} {
        std::cout << "  Widget " << id << " constructed\n";
    }

    ~Widget() {
        std::cout << "  Widget " << id << " destroyed\n";
    }
};

} // namespace

static void run_examples() {

    // create<T>() constructs objects one after another, each carved from
    // the next free bytes in the buffer.
    setTitle("Sequential Construction");

    Stack<> stack(1024);

    Widget* a = stack.create<Widget>(1);
    Widget* b = stack.create<Widget>(2);
    Widget* c = stack.create<Widget>(3);

    std::cout << "used after 3 widgets: " << stack.used() << "\n\n";

    // destroy() only runs the destructor — it has no way to give back an
    // individual allocation's bytes, so used() is unaffected.
    setTitle("Destroy Without Reclaiming");

    stack.destroy(b);

    std::cout << "used after destroying b (unchanged): " << stack.used() << "\n\n";

    // The remaining widgets are still perfectly valid — destroying one
    // object doesn't disturb its neighbors.
    setTitle("Neighbors Still Valid");

    std::cout << "a->id: " << a->id << "\n";
    std::cout << "c->id: " << c->id << "\n\n";

    // The only way to actually reclaim bytes is to roll the whole stack
    // back — either to a marker or all the way with reset(). Both require
    // destroying every live object in the reclaimed range first; here
    // nothing but b was destroyed, so a and c are cleaned up too.
    setTitle("Reclaiming Space");

    stack.destroy(a);
    stack.destroy(c);
    stack.reset();

    std::cout << "used after reset(): " << stack.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
