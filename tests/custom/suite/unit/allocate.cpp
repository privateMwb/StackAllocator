// Stack allocate() test suite.
//
// Coverage:
// - Returns a non-null pointer when capacity allows
// - Returns nullptr when the request exceeds remaining capacity
// - Successive allocations advance the offset
// - Returned pointer honors the requested alignment
// - An oversized request near SIZE_MAX fails cleanly instead of
//   overflowing the internal bounds check

#include <cstdint>
#include <support/framework.h>

using namespace StackPro;

// Verifies a request that fits in the buffer succeeds.
static void returns_pointer_within_capacity() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(16);
    CHK(p != nullptr);
}

// Verifies a request larger than the remaining capacity fails.
static void returns_nullptr_when_out_of_space() {
    Stack<> stack(8);
    std::byte* p = stack.allocate(16);
    CHK(p == nullptr);
}

// Verifies the bump-pointer offset moves forward after each allocation.
static void successive_allocations_advance_offset() {
    Stack<> stack(64);
    const std::size_t before = stack.used();

    (void)stack.allocate(8);
    const std::size_t afterFirst = stack.used();
    CHK(afterFirst > before);

    (void)stack.allocate(8);
    CHK(stack.used() > afterFirst);
}

// Verifies the returned pointer is aligned to the requested alignment.
static void respects_requested_alignment() {
    Stack<> stack(128, 64);

    // Force a misaligned offset first, then request a 16-byte-aligned block.
    (void)stack.allocate(3, 1);
    std::byte* p = stack.allocate(16, 16);

    CHK(p != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(p) % 16 == 0);
}

// Verifies a request whose size would overflow the internal bounds
// check (offset + size) fails cleanly instead of wrapping around and
// succeeding with too little real capacity.
static void oversized_request_does_not_overflow() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(SIZE_MAX - 4);
    CHK(p == nullptr);
}

// Executes all allocate() test cases.
static void run_tests() {
    RUN(returns_pointer_within_capacity);
    RUN(returns_nullptr_when_out_of_space);
    RUN(successive_allocations_advance_offset);
    RUN(respects_requested_alignment);
    RUN(oversized_request_does_not_overflow);
}

REGISTER_TEST_SUITE();
