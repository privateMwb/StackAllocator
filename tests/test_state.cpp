// Stack state management test suite.
//
// Coverage:
// - Reset clears used bytes
// - Reset restores remaining capacity
// - Reset allows reallocation
// - Reset preserves capacity

#include "test_helper.h"

using namespace AllocatorPro;

// Verifies that reset clears all allocated memory.
static void reset_clears_used() {
    Stack s{1024};
    (void)s.allocate(256);
    s.reset();
    CHK(s.used() == 0);
}

// Verifies that reset restores the allocator's remaining capacity.
static void reset_restores_remaining() {
    Stack s{1024};
    (void)s.allocate(256);
    s.reset();
    CHK(s.remaining() == s.capacity());
}

// Verifies that memory can be allocated again after a reset.
static void reset_allows_reallocation() {
    Stack s{1024};
    (void)s.allocate(1024);
    s.reset();
    std::byte* ptr = s.allocate(1024);
    CHK(ptr != nullptr);
}

// Verifies that reset preserves the allocator's total capacity.
static void reset_does_not_change_capacity() {
    Stack s{1024};
    (void)s.allocate(512);
    s.reset();
    CHK(s.capacity() == 1024);
}

// Executes all state management test cases.
void run_state_tests() {
    setTitle("State Management");

    RUN(reset_clears_used);
    RUN(reset_restores_remaining);
    RUN(reset_allows_reallocation);
    RUN(reset_does_not_change_capacity);

    std::cout << "\n";
}