// Stack scope-per-thread concurrency suite.
//
// The safe way to use Stack across threads without a shared lock is
// for each thread to own its own Stack (and StackScope) instance
// outright, with no sharing at all. This suite verifies that pattern
// holds up: independent instances never observe or interfere with
// each other, even when many run at once.
//
// Coverage:
// - Independently-owned stacks never report owning each other's
//   allocations
// - A StackScope's rollback only affects its own thread's stack
// - Many concurrent thread-local stacks each produce the same result
//   they would running alone

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

using namespace StackPro;

// Verifies concurrently-allocating, independently-owned stacks never
// cross-report ownership of each other's pointers.
TEST(ScopePerThread, StacksNoCrossSee) {
    constexpr int kThreads = 4;
    std::vector<std::unique_ptr<Stack<>>> stacks(kThreads);
    std::vector<std::byte*> pointers(kThreads, nullptr);

    for (int i = 0; i < kThreads; ++i) {
        stacks[i] = std::make_unique<Stack<>>(64);
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&stacks, &pointers, i]() { pointers[i] = stacks[i]->allocate(16); });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreads; ++i) {
        for (int j = 0; j < kThreads; ++j) {
            if (i == j)
                continue;
            EXPECT_FALSE(stacks[j]->owns(pointers[i]));
        }
    }
}

// Verifies each thread's StackScope rolls back only its own
// thread-local stack, unaffected by what other threads are doing.
TEST(ScopePerThread, RollbackPerThread) {
    constexpr int kThreads = 4;
    std::vector<std::size_t> finalUsed(kThreads, 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&finalUsed, i]() {
            Stack<> stack(64);
            (void)stack.allocate(8, 1);
            {
                StackScope<> scope(stack);
                (void)stack.allocate(16, 1);
                (void)stack.allocate(16, 1);
            }
            finalUsed[i] = stack.used();
        });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 0; i < kThreads; ++i)
        EXPECT_EQ(finalUsed[i], 8u);
}

// Verifies many concurrent thread-local stacks each end up in the same
// state a single-threaded run would produce, with no shared state
// leaking between them.
TEST(ScopePerThread, ManyStacksMatchSerial) {
    constexpr int kThreads = 16;
    std::vector<std::size_t> results(kThreads, 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&results, i]() {
            Stack<> stack(128);
            for (int j = 0; j < 10; ++j) {
                (void)stack.allocate(4, 1);
            }
            results[i] = stack.used();
        });
    }
    for (auto& t : threads)
        t.join();

    for (auto used : results)
        EXPECT_EQ(used, 40u);
}
