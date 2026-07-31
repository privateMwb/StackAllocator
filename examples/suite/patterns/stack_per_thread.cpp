// One Stack per thread.
//
// Demonstrates:
// - Stack has no internal synchronization, so sharing one across
//   threads isn't safe
// - Giving each thread its own instance instead of sharing one
// - A thread_local Stack as a reusable per-thread scratch buffer

#include <support/framework.h>

#include <thread>
#include <vector>

using namespace StackPro;

namespace {

struct WorkItem {
    int thread_index;
    int value;
};

// Each thread gets its own Stack, constructed on that thread's own call
// stack — nothing here is shared, so there's nothing to race on.
void runOnOwnStack(int thread_index, int allocations) {
    Stack<> local(4096);

    for (int i = 0; i < allocations; ++i) {
        (void)local.create<WorkItem>(thread_index, i);
    }

    std::cout << "thread " << thread_index << " used " << local.used() << " bytes, alone\n";
}

// A function-local thread_local Stack persists across calls on the same
// thread and is reused rather than rebuilt each time — useful when a
// hot function is called repeatedly from several worker threads.
Stack<>& perThreadScratch() {
    thread_local Stack<> scratch(4096);
    return scratch;
}

void useThreadLocalScratch(int call_index) {
    StackScope<> scope(perThreadScratch());
    (void)perThreadScratch().create<WorkItem>(call_index, call_index * 10);
}

} // namespace

static void run_examples() {

    // Give each thread an independent Stack — no shared state, so no
    // locking is needed and none of Stack's methods have to be
    // thread-safe for this to work correctly.
    setTitle("One Stack Per Thread");

    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back(runOnOwnStack, i, 5 + i);
    }
    for (auto& t : workers) {
        t.join();
    }
    std::cout << "\n";

    // A thread_local Stack behind an accessor function gives every
    // thread its own persistent scratch buffer, reused call after call
    // instead of being constructed fresh each time.
    setTitle("Thread-Local Scratch, Reused Across Calls");

    std::vector<std::thread> callers;
    for (int i = 0; i < 4; ++i) {
        callers.emplace_back([i] {
            for (int call = 0; call < 3; ++call) {
                useThreadLocalScratch(i * 10 + call);
            }
            std::cout << "thread " << i << " finished its calls\n";
        });
    }
    for (auto& t : callers) {
        t.join();
    }
}

REGISTER_EXAMPLE_SUITE();
