// Scoped temporary allocations with StackScope.
//
// Demonstrates:
// - Using StackScope instead of manual getMarker()/freeToMarker() pairs
// - Automatic rollback on scope exit, including early returns
// - Nesting scopes inside a loop for per-iteration scratch space

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Row {
    int id;
};

bool isValid(int id) {
    return id % 2 == 0;
}

} // namespace

static void run_examples() {

    // StackScope captures a marker on construction and rolls back to it
    // on destruction — the manual pattern from marker_rollback.cpp, but
    // impossible to forget.
    setTitle("A Scope That Cleans Up Itself");

    Stack<> stack(1024);

    {
        StackScope<> scope(stack);
        (void)stack.create<Row>(1);
        (void)stack.create<Row>(2);
        std::cout << "used inside scope: " << stack.used() << "\n";
    }

    std::cout << "used after scope exit: " << stack.used() << "\n\n";

    // The real value shows up with early returns or exceptions — every
    // exit path runs the destructor, so there's no cleanup to forget.
    setTitle("Cleanup on Every Exit Path");

    auto process = [&stack](int id) {
        StackScope<> scope(stack);
        Row* row = stack.create<Row>(id);

        if (!isValid(row->id)) {
            std::cout << "  early return for id " << row->id << ", scope still unwinds\n";
            return; // scope's destructor still runs here
        }

        std::cout << "  processed id " << row->id << "\n";
    };

    process(1); // invalid, returns early
    process(2); // valid, falls through

    std::cout << "used after both calls: " << stack.used() << "\n\n";

    // Scopes nest naturally in a loop — each iteration's scratch space
    // is reclaimed before the next one starts, so working memory never
    // accumulates across iterations.
    setTitle("Per-Iteration Scratch in a Loop");

    for (int i = 0; i < 3; ++i) {
        StackScope<> scope(stack);
        (void)stack.create<Row>(i);
        std::cout << "  iteration " << i << ", used: " << stack.used() << "\n";
    }

    std::cout << "used after the loop: " << stack.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
