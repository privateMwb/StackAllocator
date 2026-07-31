// Ignoring a failed allocation.
//
// Demonstrates:
// - allocate() returning nullptr once capacity is exhausted
// - Why that nullptr must be checked before use
// - What an unchecked failure looks like at the call site

#include <support/framework.h>

using namespace StackPro;

static void run_examples() {

    // A small stack makes exhaustion easy to reach on purpose.
    setTitle("Filling the Stack");

    Stack<> stack(64);

    std::byte* first = stack.allocate(64);
    std::cout << "first allocate(64) succeeded: " << (first != nullptr) << "\n";
    std::cout << "remaining: " << stack.remaining() << "\n\n";

    // There's no room left, so the next request fails cleanly —
    // allocate() never throws, it just says no.
    setTitle("The Next Request Fails");

    std::byte* second = stack.allocate(16);
    std::cout << "second allocate(16) succeeded: " << (second != nullptr) << "\n\n";

    // The bug is what happens if that result isn't checked. Writing
    // through `second` here would be a null-pointer write — undefined
    // behavior, not something allocate() can protect against, since it
    // already told the caller it failed. Left commented out on purpose:
    // this would crash.
    setTitle("The Unchecked Version Crashes");

    std::cout << "// *second = std::byte{0}; <- would dereference nullptr\n";
    std::cout << "allocate()'s [[nodiscard]] return exists to make skipping\n";
    std::cout << "this check a compiler warning, not just a hope.\n\n";

    // The correct pattern: branch on the result before touching it.
    setTitle("Checking Before Use");

    if (second != nullptr) {
        std::cout << "would use second here\n";
    } else {
        std::cout << "handled gracefully: fall back, grow the stack, or fail the caller\n";
    }
}

REGISTER_EXAMPLE_SUITE();
