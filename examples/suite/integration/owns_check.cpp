// Validating pointer provenance with owns().
//
// Demonstrates:
// - Using owns() to confirm a pointer came from a particular stack
// - Rejecting pointers from another stack or from the heap
// - A gate function that only trusts pointers a stack actually issued

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Payload {
    int id;
};

// A subsystem boundary that accepts raw pointers from callers but only
// operates on ones it can prove came from its own stack.
class Registry {
  public:
    explicit Registry(Stack<>& stack) : stack_(stack) {}

    bool accept(const void* ptr) const {
        return stack_.owns(ptr);
    }

  private:
    Stack<>& stack_;
};

} // namespace

static void run_examples() {

    // owns() answers a narrow question: is this address inside my buffer,
    // regardless of what's stored there or whether it's still "live".
    setTitle("Checking Ownership");

    Stack<> stack(1024);
    Payload* mine = stack.create<Payload>(1);

    std::cout << "stack owns its own pointer: " << stack.owns(mine) << "\n\n";

    // A pointer from a different stack entirely is correctly rejected,
    // even though it points to the same kind of object.
    setTitle("Rejecting a Foreign Stack's Pointer");

    Stack<> other(1024);
    Payload* theirs = other.create<Payload>(2);

    std::cout << "stack owns other's pointer: " << stack.owns(theirs) << "\n\n";

    // A plain heap pointer is rejected the same way — owns() only cares
    // about address ranges, not allocation history.
    setTitle("Rejecting a Heap Pointer");

    Payload* heap_ptr = new Payload{3};

    std::cout << "stack owns a heap pointer: " << stack.owns(heap_ptr) << "\n\n";
    delete heap_ptr;

    // A Registry built around owns() gives a subsystem a way to gate
    // input without trusting callers to tag their pointers correctly.
    setTitle("Gating Input at a Subsystem Boundary");

    Registry registry(stack);
    std::cout << "registry accepts stack's own pointer: " << registry.accept(mine) << "\n";
    std::cout << "registry accepts other's pointer     : " << registry.accept(theirs) << "\n";

    stack.destroy(mine);
    other.destroy(theirs);
}

REGISTER_EXAMPLE_SUITE();
