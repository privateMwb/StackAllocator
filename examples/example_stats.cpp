// Stack statistics example.
//
// Demonstrates:
// - Allocation statistics tracking
// - Peak usage tracking
// - Statistics after marker restoration
// - Statistics after reset

#include "example_helper.h"

using namespace AllocatorPro;

int main() {
    mainTitle("\nStats Examples");
    borderLine();

    Stack<true> s{1024};

    // Initial statistics.
    setTitle("Initial Statistics");

    std::cout << "Total allocated : " << s.getStats().totalAllocated_ << "\n";
    std::cout << "Current used    : " << s.getStats().currentUsed_    << "\n";
    std::cout << "Peak used       : " << s.getStats().peakUsed_       << "\n";
    std::cout << "Allocations     : " << s.getStats().allocations_    << "\n\n";

    // Allocation statistics tracking.
    setTitle("Tracking Allocations");

    (void)s.allocate(128);
    std::cout << "After 128 byte allocation:\n";
    std::cout << "  Total allocated : " << s.getStats().totalAllocated_ << "\n";
    std::cout << "  Current used    : " << s.getStats().currentUsed_    << "\n";
    std::cout << "  Peak used       : " << s.getStats().peakUsed_       << "\n";
    std::cout << "  Allocations     : " << s.getStats().allocations_    << "\n\n";

    (void)s.allocate(256);
    std::cout << "After 256 byte allocation:\n";
    std::cout << "  Total allocated : " << s.getStats().totalAllocated_ << "\n";
    std::cout << "  Current used    : " << s.getStats().currentUsed_    << "\n";
    std::cout << "  Peak used       : " << s.getStats().peakUsed_       << "\n";
    std::cout << "  Allocations     : " << s.getStats().allocations_    << "\n\n";

    // Peak usage tracking.
    setTitle("Peak Used Tracking");

    auto marker = s.getMarker();
    (void)s.allocate(512);
    std::cout << "Peak after 512 byte allocation : " << s.getStats().peakUsed_ << "\n";

    s.freeToMarker(marker);
    std::cout << "Peak after freeToMarker        : " << s.getStats().peakUsed_ << "\n";
    std::cout << "Current used after free        : " << s.getStats().currentUsed_ << "\n\n";

    // Statistics after reset.
    setTitle("Stats After Reset");

    std::cout << "Total allocated before reset : " << s.getStats().totalAllocated_ << "\n";
    std::cout << "Allocations before reset     : " << s.getStats().allocations_    << "\n";

    s.reset();

    std::cout << "Total allocated after reset  : " << s.getStats().totalAllocated_ << "\n";
    std::cout << "Current used after reset     : " << s.getStats().currentUsed_    << "\n";
    std::cout << "Peak used after reset        : " << s.getStats().peakUsed_       << "\n";
    std::cout << "Allocations after reset      : " << s.getStats().allocations_    << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}