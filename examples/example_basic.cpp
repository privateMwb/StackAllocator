// Basic Stack example.
//
// Demonstrates:
// - Stack construction
// - Raw memory allocation
// - Marker capture and restoration
// - Stack reset
// - Stack introspection

#include "example_helper.h"

using namespace AllocatorPro;

int main() {
    mainTitle("\nBasic Examples");
    borderLine();

    // Stack construction.
    setTitle("Construction");

    Stack s{1024};

    std::cout << "Capacity  : " << s.capacity()  << "\n";
    std::cout << "Used      : " << s.used()      << "\n";
    std::cout << "Remaining : " << s.remaining() << "\n\n";

    // Raw memory allocation.
    setTitle("Raw Allocation");

    std::byte* a = s.allocate(128);
    std::byte* b = s.allocate(256);
    std::byte* c = s.allocate(64);
    (void)a;
    (void)b;
    (void)c;

    std::cout << "Used after 3 allocations      : " << s.used()      << "\n";
    std::cout << "Remaining after 3 allocations : " << s.remaining() << "\n\n";

    // Marker capture and restoration.
    setTitle("Marker Capture And Restore");

    s.reset();
    (void)s.allocate(128);

    auto marker = s.getMarker();
    std::cout << "Marker captured at          : " << marker.get() << "\n";

    (void)s.allocate(256);
    std::cout << "Used after extra allocation : " << s.used() << "\n";

    s.freeToMarker(marker);
    std::cout << "Used after freeToMarker     : " << s.used() << "\n\n";

    // Stack introspection.
    setTitle("Introspection");

    std::byte* ptr = s.allocate(64);

    std::cout << "Owns ptr  : " << s.owns(ptr)     << "\n";

    int x = 0;
    std::cout << "Owns &x   : " << s.owns(&x)      << "\n";
    std::cout << "Owns null : " << s.owns(nullptr) << "\n\n";

    // Reset the allocator.
    setTitle("Reset");

    std::cout << "Used before reset     : " << s.used() << "\n";
    s.reset();
    std::cout << "Used after reset      : " << s.used() << "\n";
    std::cout << "Remaining after reset : " << s.remaining() << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}

