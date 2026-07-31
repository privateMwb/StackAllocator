// Holding a pointer past a rollback.
//
// Demonstrates:
// - A pointer surviving freeToMarker() as a plain address
// - Why using it afterward is reading a dead object, not a live one
// - How a later allocation can silently reuse and overwrite those bytes

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Token {
    int value;
};

} // namespace

static void run_examples() {

    // freeToMarker() doesn't clear or poison memory — it only moves the
    // offset back. The old pointer still "looks" valid.
    setTitle("A Pointer That Outlives Its Rollback");

    Stack<> stack(1024);

    Stack<>::Marker checkpoint = stack.getMarker();
    Token* token = stack.create<Token>(42);

    std::cout << "token->value before rollback: " << token->value << "\n";

    stack.freeToMarker(checkpoint);
    std::cout << "rolled back — token's storage is now unowned by any object\n\n";

    // token is now dangling in the object-lifetime sense: its bytes are
    // still mapped and readable, but ~Token() was never run and nothing
    // constructed there anymore. Reading token->value here happens to
    // print the old bits, but that's luck, not a guarantee.
    setTitle("Reading a Dangling Pointer (Undefined Behavior)");

    std::cout << "token->value still reads: " << token->value << " (don't rely on this)\n\n";

    // The real danger shows up once new allocations reuse those same
    // bytes for something else entirely.
    setTitle("The Bytes Get Reused");

    Token* fresh = stack.create<Token>(99);
    std::cout << "fresh->value: " << fresh->value << "\n";
    std::cout << "token->value now reads: " << token->value << " — same address as fresh\n\n";

    // The fix: treat every pointer as invalid the moment its marker (or
    // an earlier one) is rolled back, the same way you'd treat a pointer
    // as invalid after delete.
    setTitle("Avoiding It");

    std::cout << "drop or null out pointers at the same point you call\n";
    std::cout << "freeToMarker() — don't let them outlive the rollback.\n";
}

REGISTER_EXAMPLE_SUITE();
