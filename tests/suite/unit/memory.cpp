// Stack memory management test suite.
//
// Coverage:
// - Basic allocation
// - Allocation updates used and remaining
// - Allocation with custom alignment
// - Allocation returns aligned pointer
// - Exhaustion returns nullptr
// - Get marker
// - Free to marker restores allocation state
// - Nested markers

#include <common/framework.h>

using namespace AllocatorPro;

// Verifies that a basic allocation returns a non-null pointer.
static void basic_allocation() {
    Stack s{1024};
    std::byte* ptr = s.allocate(64);
    CHK(ptr != nullptr);
}

// Verifies that allocation updates the allocator's usage statistics.
static void allocation_updates_state() {
    Stack s{1024};
    (void)s.allocate(64);
    CHK(s.used()      == 64);
    CHK(s.remaining() == 960);
}

// Verifies that allocation honors a custom power-of-two alignment.
static void allocation_custom_alignment() {
    Stack s{1024};
    std::byte* ptr = s.allocate(64, 16);
    CHK(ptr != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(ptr) % 16 == 0);
}

// Verifies that the returned pointer satisfies the default alignment.
static void allocation_default_alignment() {
    Stack s{1024};
    std::byte* ptr = s.allocate(64);
    CHK(reinterpret_cast<std::uintptr_t>(ptr) % alignof(std::max_align_t) == 0);
}

// Verifies that allocation beyond the allocator's capacity returns nullptr.
static void exhaustion_returns_nullptr() {
    Stack s{64};
    (void)s.allocate(64);
    std::byte* ptr = s.allocate(1);
    CHK(ptr == nullptr);
}

// Verifies that getMarker returns the current allocation checkpoint.
static void get_marker() {
    Stack s{1024};
    (void)s.allocate(64);
    auto marker = s.getMarker();
    CHK(marker.get() == 64);
}

// Verifies that freeToMarker restores the allocator to a previous checkpoint.
static void free_to_marker() {
    Stack s{1024};
    auto marker = s.getMarker();
    (void)s.allocate(256);
    s.freeToMarker(marker);
    CHK(s.used()      == 0);
    CHK(s.remaining() == 1024);
}

// Verifies that multiple checkpoints are restored in LIFO order.
static void nested_markers() {
    Stack s{1024};
    auto m0 = s.getMarker();
    (void)s.allocate(128);
    auto m1 = s.getMarker();
    (void)s.allocate(128);

    s.freeToMarker(m1);
    CHK(s.used() == 128);

    s.freeToMarker(m0);
    CHK(s.used() == 0);
}

// Executes all memory management test cases.
static void run_tests() {
    RUN(basic_allocation);
    RUN(allocation_updates_state);
    RUN(allocation_custom_alignment);
    RUN(allocation_default_alignment);
    RUN(exhaustion_returns_nullptr);
    RUN(get_marker);
    RUN(free_to_marker);
    RUN(nested_markers);
}

REGISTER_TEST_SUITE();