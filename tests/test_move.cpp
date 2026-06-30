// Stack move semantics test suite.
//
// Coverage:
// - Move constructor
// - Move assignment
// - Self move-assignment

#include "test_helper.h"

using namespace AllocatorPro;

// Verifies that move construction transfers ownership of the allocator state.
static void move_constructor() {
    Stack src{1024};
    (void)src.allocate(128);

    Stack dst{std::move(src)};

    CHK(dst.capacity()  == 1024);
    CHK(dst.used()      == 128);
    CHK(src.capacity()  == 0);
    CHK(src.used()      == 0);
}

// Verifies that move assignment transfers ownership of the allocator state.
static void move_assignment() {
    Stack src{1024};
    Stack dst{512};

    (void)src.allocate(128);
    dst = std::move(src);

    CHK(dst.capacity()  == 1024);
    CHK(dst.used()      == 128);
    CHK(src.capacity()  == 0);
    CHK(src.used()      == 0);
}

// Verifies that self move-assignment preserves a valid allocator state.
static void move_self_assignment() {
    Stack s{1024};
    (void)s.allocate(128);

    s = std::move(s);

    CHK(s.capacity() == 1024);
    CHK(s.used()     == 128);
}

// Executes all move semantics test cases.
void run_move_tests() {
    setTitle("Move Semantics");

    RUN(move_constructor);
    RUN(move_assignment);
    RUN(move_self_assignment);

    std::cout << "\n";
}