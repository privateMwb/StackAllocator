// Stack object lifecycle test suite.
//
// Coverage:
// - Create returns non-null pointer
// - Create constructs object correctly
// - Create with arguments
// - Create respects alignment
// - Create returns nullptr on exhaustion
// - Destroy calls destructor
// - Destroy null pointer is safe

#include "test_helper.h"

using namespace AllocatorPro;

// Tracks destructor invocations for lifetime verification.
struct Tracker {
    int  value = 0;
    int& count;
    explicit Tracker(int val, int& count) : value{val}, count{count} {}
    ~Tracker() { ++count; }
};

// Verifies that create returns a valid pointer on success.
static void create_returns_nonnull() {
    Stack s{1024};
    int* ptr = s.create<int>(42);
    CHK(ptr != nullptr);
}

// Verifies that create constructs an object with the expected value.
static void create_constructs_correctly() {
    Stack s{1024};
    int* ptr = s.create<int>(42);
    CHK(*ptr == 42);
}

// Verifies that create forwards constructor arguments correctly.
static void create_with_arguments() {
    Stack s{1024};
    int destructCount = 0;
    Tracker* t = s.create<Tracker>(99, destructCount);
    CHK(t         != nullptr);
    CHK(t->value  == 99);
}

// Verifies that created objects satisfy their required alignment.
static void create_respects_alignment() {
    Stack s{1024};
    double* ptr = s.create<double>(3.14);
    CHK(reinterpret_cast<std::uintptr_t>(ptr) % alignof(double) == 0);
}

// Verifies that create returns nullptr when insufficient memory is available.
static void create_returns_nullptr_on_exhaustion() {
    Stack s{4};
    (void)s.allocate(4);
    int* ptr = s.create<int>(1);
    CHK(ptr == nullptr);
}

// Verifies that destroy invokes the object's destructor exactly once.
static void destroy_calls_destructor() {
    Stack s{1024};
    int destructCount = 0;
    Tracker* t = s.create<Tracker>(1, destructCount);
    s.destroy(t);
    CHK(destructCount == 1);
}

// Verifies that destroying a null pointer has no effect.
static void destroy_null_is_safe() {
    Stack s{1024};
    int* ptr = nullptr;
    s.destroy(ptr);
    CHK(true);
}

// Executes all object lifecycle test cases.
void run_lifecycle_tests() {
    setTitle("Object Lifecycle");

    RUN(create_returns_nonnull);
    RUN(create_constructs_correctly);
    RUN(create_with_arguments);
    RUN(create_respects_alignment);
    RUN(create_returns_nullptr_on_exhaustion);
    RUN(destroy_calls_destructor);
    RUN(destroy_null_is_safe);

    std::cout << "\n";
}