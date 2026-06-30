// Stack introspection test suite.
//
// Coverage:
// - Owns returns true for allocated pointer
// - Owns returns true for interior pointer
// - Owns returns false for outside pointer
// - Owns returns false for nullptr
// - Used reflects current allocation
// - Remaining reflects available capacity
// - Capacity reflects total size
// - Used and remaining sum to capacity

#include "test_helper.h"

using namespace AllocatorPro;

// Verifies that owns returns true for an allocated pointer.
static void owns_allocated_pointer() {
    Stack s{1024};
    std::byte* ptr = s.allocate(64);
    CHK(s.owns(ptr));
}

// Verifies that owns returns true for a pointer within an allocated block.
static void owns_interior_pointer() {
    Stack s{1024};
    std::byte* ptr = s.allocate(64);
    CHK(s.owns(ptr + 32));
}

// Verifies that owns returns false for a pointer outside the allocator.
static void owns_outside_pointer() {
    Stack s{1024};
    int x = 42;
    CHK(!s.owns(&x));
}

// Verifies that owns returns false for a null pointer.
static void owns_nullptr() {
    Stack s{1024};
    CHK(!s.owns(nullptr));
}

// Verifies that used reports the current allocation size.
static void used_reflects_offset() {
    Stack s{1024};
    (void)s.allocate(128);
    (void)s.allocate(256);
    CHK(s.used() == 384);
}

// Verifies that remaining reports the available capacity.
static void remaining_reflects_available() {
    Stack s{1024};
    (void)s.allocate(128);
    CHK(s.remaining() == 896);
}

// Verifies that capacity reports the allocator's total capacity.
static void capacity_reflects_total_size() {
    Stack s{1024};
    (void)s.allocate(512);
    CHK(s.capacity() == 1024);
}

// Verifies that used and remaining always equal the total capacity.
static void used_and_remaining_sum_to_capacity() {
    Stack s{1024};
    (void)s.allocate(300);
    CHK(s.used() + s.remaining() == s.capacity());
}

// Executes all introspection test cases.
void run_introspection_tests() {
    setTitle("Introspection");

    RUN(owns_allocated_pointer);
    RUN(owns_interior_pointer);
    RUN(owns_outside_pointer);
    RUN(owns_nullptr);
    RUN(used_reflects_offset);
    RUN(remaining_reflects_available);
    RUN(capacity_reflects_total_size);
    RUN(used_and_remaining_sum_to_capacity);

    std::cout << "\n";
}