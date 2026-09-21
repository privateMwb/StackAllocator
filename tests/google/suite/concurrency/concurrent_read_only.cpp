// Stack concurrent-read-only-observers concurrency suite.
//
// While mutating calls need external locking, the const observers
// (used(), remaining(), capacity(), owns(), getStats()) only read
// already-settled state. With no writer active concurrently, many
// threads calling them at once is well-defined and should always see
// the same, consistent snapshot.
//
// Coverage:
// - used()/remaining()/capacity() agree across many concurrent readers
// - owns() gives the same answer to every concurrent reader
// - getStats() gives the same answer to every concurrent reader

#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>
#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace StackPro;

// Verifies used(), remaining(), and capacity() are stable across
// concurrent, lock-free readers.
TEST(ConcurrentReadOnly, ReadsConsistentSnapshot) {
    Stack<> stack(128);
    (void)stack.allocate(32, 1);

    const std::size_t expectedUsed = stack.used();
    const std::size_t expectedRemaining = stack.remaining();
    const std::size_t expectedCapacity = stack.capacity();
    std::atomic<bool> mismatch{false};

    auto worker = [&]() {
        for (int i = 0; i < 1000; ++i) {
            if (stack.used() != expectedUsed)
                mismatch = true;
            if (stack.remaining() != expectedRemaining)
                mismatch = true;
            if (stack.capacity() != expectedCapacity)
                mismatch = true;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    EXPECT_FALSE(mismatch.load());
}

// Verifies owns() gives every concurrent reader the same answer for a
// fixed, live pointer.
TEST(ConcurrentReadOnly, OwnsCheckConsistent) {
    Stack<> stack(64);
    std::byte* p = stack.allocate(16);
    std::atomic<int> ownedCount{0};

    auto worker = [&]() {
        for (int i = 0; i < 1000; ++i) {
            if (stack.owns(p))
                ++ownedCount;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    EXPECT_EQ(ownedCount.load(), 8 * 1000);
}

// Verifies getStats() gives every concurrent reader the same totals.
TEST(ConcurrentReadOnly, StatsReadConsistent) {
    Stack<true> stack(64);
    (void)stack.allocate(16, 1);
    (void)stack.allocate(8, 1);

    const std::size_t expectedTotal = stack.getStats().totalAllocated_;
    std::atomic<bool> mismatch{false};

    auto worker = [&]() {
        for (int i = 0; i < 1000; ++i) {
            if (stack.getStats().totalAllocated_ != expectedTotal)
                mismatch = true;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    EXPECT_FALSE(mismatch.load());
}
