// Stack external-locking-contract concurrency suite.
//
// Stack has no internal synchronization of its own — it's the caller's
// job to serialize mutating calls (allocate(), create(), destroy(),
// freeToMarker(), reset()) across threads, typically with a mutex
// owned alongside the Stack instance. This suite verifies that once
// that external lock is in place, concurrent use behaves exactly like
// single-threaded use.
//
// Coverage:
// - Concurrent, lock-serialized allocations never hand out overlapping
//   memory ranges
// - used() after all threads join equals exactly the byte total of the
//   allocations that actually succeeded, even when the buffer is too
//   small for every request
// - A marker taken before concurrent activity still rolls back
//   everything those threads allocated

#include <algorithm>
#include <mutex>
#include <support/framework.h>
#include <thread>
#include <vector>

using namespace StackPro;

// Verifies lock-serialized concurrent allocations never overlap.
static void concurrent_allocations_do_not_overlap() {
    constexpr int kThreads = 8;
    constexpr int kAllocsPerThread = 20;
    constexpr std::size_t kAllocSize = 16;

    Stack<> stack(kThreads * kAllocsPerThread * kAllocSize);
    std::mutex lock;
    std::vector<std::byte*> pointers;

    auto worker = [&]() {
        for (int i = 0; i < kAllocsPerThread; ++i) {
            std::lock_guard<std::mutex> guard(lock);
            std::byte* p = stack.allocate(kAllocSize, 1);
            if (p)
                pointers.push_back(p);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    CHK(pointers.size() == static_cast<std::size_t>(kThreads * kAllocsPerThread));

    std::sort(pointers.begin(), pointers.end());
    bool overlapFree = true;
    for (std::size_t i = 1; i < pointers.size(); ++i) {
        if (pointers[i] < pointers[i - 1] + kAllocSize) {
            overlapFree = false;
            break;
        }
    }
    CHK(overlapFree);
}

// Verifies used() after joining matches exactly the successful
// allocation count, even when threads race for a buffer too small to
// satisfy every request.
static void total_used_matches_successful_allocation_count() {
    constexpr int kThreads = 4;
    constexpr int kAllocsPerThread = 50;
    constexpr std::size_t kAllocSize = 8;

    Stack<> stack(kAllocSize * (kThreads * kAllocsPerThread / 2));
    std::mutex lock;
    std::size_t successes = 0;

    auto worker = [&]() {
        for (int i = 0; i < kAllocsPerThread; ++i) {
            std::lock_guard<std::mutex> guard(lock);
            if (stack.allocate(kAllocSize, 1) != nullptr)
                ++successes;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    CHK(stack.used() == successes * kAllocSize);
}

// Verifies a marker taken before concurrent activity rolls back
// everything allocated by every thread under the shared lock.
static void free_to_marker_under_lock_is_consistent() {
    Stack<> stack(1024);
    std::mutex lock;
    auto marker = stack.getMarker();

    auto worker = [&]() {
        for (int i = 0; i < 50; ++i) {
            std::lock_guard<std::mutex> guard(lock);
            (void)stack.allocate(4, 1);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    CHK(stack.used() == 4 * 50 * 4);

    {
        std::lock_guard<std::mutex> guard(lock);
        stack.freeToMarker(marker);
    }
    CHK(stack.used() == marker.get());
}

// Executes all external-locking-contract test cases.
static void run_tests() {
    RUN(concurrent_allocations_do_not_overlap);
    RUN(total_used_matches_successful_allocation_count);
    RUN(free_to_marker_under_lock_is_consistent);
}

REGISTER_TEST_SUITE();
