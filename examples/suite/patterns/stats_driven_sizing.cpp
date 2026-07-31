// Sizing a production stack from measured peak usage.
//
// Demonstrates:
// - Running representative workloads through a Stack<true> to measure
//   peakUsed_
// - Using that number, plus headroom, to size the real Stack<>
// - Why this beats guessing a buffer size up front

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Job {
    int id;
    double payload[4];

    Job(int id, double fill) : id{id}, payload{fill, fill, fill, fill} {}
};

// A stand-in for "do the real work" — allocates however much a given
// workload actually needs, without the caller having to know in advance.
void runWorkload(Stack<true>& stack, int job_count) {
    StackScope<true> scope(stack);
    for (int i = 0; i < job_count; ++i) {
        (void)stack.create<Job>(i, 0.0);
    }
}

} // namespace

static void run_examples() {

    // Measure first: run the workloads that matter against an
    // instrumented, generously-sized stack.
    setTitle("Measuring Peak Usage");

    Stack<true> probe(1 << 20); // 1 MiB, comfortably oversized for measurement

    runWorkload(probe, 10);
    runWorkload(probe, 50); // the largest workload seen
    runWorkload(probe, 20);

    std::size_t measured_peak = probe.getStats().peakUsed_;
    std::cout << "measured peakUsed_: " << measured_peak << " bytes\n\n";

    // Size the production stack from that number, with headroom for
    // workloads slightly larger than anything measured — not from a
    // round number picked without evidence.
    setTitle("Sizing the Production Stack");

    constexpr double headroom = 1.25; // 25% margin over the worst case seen
    std::size_t production_size = static_cast<std::size_t>(measured_peak * headroom);

    std::cout << "production stack size: " << production_size << " bytes\n\n";

    Stack<> production(production_size);
    std::cout << "production capacity: " << production.capacity() << "\n";
}

REGISTER_EXAMPLE_SUITE();
