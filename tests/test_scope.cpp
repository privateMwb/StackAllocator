// StackScope test suite.
//
// Coverage:
// - Scope restores allocation state on destruction
// - Scope preserves prior allocations
// - Nested scopes unwind in LIFO order
// - Scope allows reallocation after destruction
// - Empty scope is a no-op

#include "test_helper.h"

using namespace AllocatorPro;

// Verifies that a scope restores the allocator state when destroyed.
static void scope_restores_offset() {
    Stack s{1024};
    (void)s.allocate(128);

    {
        StackScope scope{s};
        (void)s.allocate(256);
        CHK(s.used() == 384);
    }

    CHK(s.used() == 128);
}

// Verifies that allocations made before the scope remain valid.
static void scope_does_not_affect_prior_allocations() {
    Stack s{1024};
    std::byte* ptr = s.allocate(128);

    {
        StackScope scope{s};
        (void)s.allocate(256);
    }

    CHK(s.owns(ptr));
    CHK(s.used() == 128);
}

// Verifies that nested scopes restore the allocator state in LIFO order.
static void nested_scopes_unwind_lifo() {
    Stack s{1024};

    {
        StackScope outer{s};
        (void)s.allocate(128);

        {
            StackScope inner{s};
            (void)s.allocate(128);
            CHK(s.used() == 256);
        }

        CHK(s.used() == 128);
    }

    CHK(s.used() == 0);
}

// Verifies that memory released by a scope can be allocated again.
static void scope_allows_reallocation() {
    Stack s{1024};

    {
        StackScope scope{s};
        (void)s.allocate(512);
    }

    std::byte* ptr = s.allocate(512);
    CHK(ptr != nullptr);
    CHK(s.used() == 512);
}

// Verifies that an empty scope leaves the allocator unchanged.
static void empty_scope_is_noop() {
    Stack s{1024};
    (void)s.allocate(128);

    {
        StackScope scope{s};
    }

    CHK(s.used() == 128);
}

// Executes all StackScope test cases.
void run_scope_tests() {
    setTitle("StackScope");

    RUN(scope_restores_offset);
    RUN(scope_does_not_affect_prior_allocations);
    RUN(nested_scopes_unwind_lifo);
    RUN(scope_allows_reallocation);
    RUN(empty_scope_is_noop);

    std::cout << "\n";
}