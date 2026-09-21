// Stack move semantics test suite.
//
// Coverage:
// - Move construction transfers capacity/usage state and empties the source
// - Move assignment releases the destination's old buffer and adopts
//   the source's state, emptying the source
// - A moved-from stack is safe to keep using (allocate() just fails)
// - Self-move-assignment is a no-op that leaves state untouched

#include <support/framework.h>

using namespace StackPro;

// Verifies move construction transfers state and leaves the source empty.
static void move_construction_transfers_state() {
    Stack<> source(64);
    (void)source.allocate(16);

    Stack<> dest(std::move(source));

    CHK(dest.capacity() == 64);
    CHK(dest.used() == 16);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(source.capacity() == 0);
    CHK(source.used() == 0);
}

// Verifies move assignment releases the destination's prior buffer and
// adopts the source's state, leaving the source empty.
static void move_assignment_adopts_source_state() {
    Stack<> dest(32);
    (void)dest.allocate(8);

    Stack<> source(64);
    (void)source.allocate(16);

    dest = std::move(source);

    CHK(dest.capacity() == 64);
    CHK(dest.used() == 16);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(source.capacity() == 0);
    CHK(source.used() == 0);
}

// Verifies a moved-from stack is safe to keep calling: allocate() just
// fails cleanly since its capacity is now zero.
static void moved_from_stack_is_safe_to_use() {
    Stack<> source(64);
    Stack<> dest(std::move(source));

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    std::byte* p = source.allocate(1);

    CHK(p == nullptr);
    CHK(source.used() == 0);
    CHK(source.remaining() == 0);
}

// Verifies self-move-assignment leaves the stack's state untouched.
static void self_move_assignment_is_noop() {
    Stack<> stack(64);
    (void)stack.allocate(16);

    Stack<>* self = &stack;
    stack = std::move(*self);

    CHK(stack.capacity() == 64);
    CHK(stack.used() == 16);
}

// Executes all move semantics test cases.
static void run_tests() {
    RUN(move_construction_transfers_state);
    RUN(move_assignment_adopts_source_state);
    RUN(moved_from_stack_is_safe_to_use);
    RUN(self_move_assignment_is_noop);
}

REGISTER_TEST_SUITE();
