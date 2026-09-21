// Stack alignment test suite.
//
// Coverage:
// - Default allocate() alignment matches alignof(std::max_align_t)
// - A custom construction alignment is honored by allocate()
// - A stricter per-call alignment consumes padding bytes as needed

#include <cstdint>
#include <support/framework.h>

using namespace StackPro;

// Verifies allocate() defaults to alignof(std::max_align_t) when no
// alignment is requested.
static void default_allocate_alignment() {
    Stack<> stack(64);
    std::byte* p = stack.allocate(8);

    CHK(p != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(p) % alignof(std::max_align_t) == 0);
}

// Verifies a stricter construction-time alignment is honored by
// subsequent allocations that request it.
static void custom_construction_alignment_honored() {
    Stack<> stack(128, 64);
    std::byte* p = stack.allocate(8, 64);

    CHK(p != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(p) % 64 == 0);
}

// Verifies a misaligned offset is padded forward to satisfy a stricter
// per-call alignment request, consuming more than just the requested size.
static void padding_consumes_capacity() {
    Stack<> stack(64);
    (void)stack.allocate(1, 1);
    const std::size_t usedAfterSmall = stack.used();

    (void)stack.allocate(8, 8);

    CHK(stack.used() > usedAfterSmall + 8);
}

// Executes all alignment test cases.
static void run_tests() {
    RUN(default_allocate_alignment);
    RUN(custom_construction_alignment_honored);
    RUN(padding_consumes_capacity);
}

REGISTER_TEST_SUITE();
