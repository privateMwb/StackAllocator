// Bulk-allocating structs until the stack runs out.
//
// Demonstrates:
// - Looping create<T>() until it returns nullptr
// - Tracking how many objects actually fit
// - Why this pattern needs no per-object bookkeeping to unwind cleanly

#include <support/framework.h>

#include <vector>

using namespace StackPro;

namespace {

struct Particle {
    float x, y, z;
    float lifetime;
};

} // namespace

static void run_examples() {

    // Deliberately small, so exhaustion happens within a few dozen
    // objects instead of requiring a huge loop to demonstrate.
    setTitle("Allocating Until Exhaustion");

    Stack<> stack(512);

    std::vector<Particle*> particles;
    while (Particle* p = stack.create<Particle>(0.0f, 0.0f, 0.0f, 1.0f)) {
        particles.push_back(p);
    }

    std::cout << "particles allocated: " << particles.size() << "\n";
    std::cout << "bytes used          : " << stack.used() << "\n";
    std::cout << "bytes remaining     : " << stack.remaining() << "\n\n";

    // Each Particle is trivially destructible, so there's nothing to run
    // per-object — but destroy() is still the documented way to end each
    // one's lifetime before the storage is reclaimed.
    setTitle("Tearing It Down");

    for (Particle* p : particles) {
        stack.destroy(p);
    }
    particles.clear();

    std::cout << "used after destroying all (unchanged): " << stack.used() << "\n\n";

    // reset() reclaims every byte in one call — no need to track markers
    // or free particles one at a time to get the space back.
    setTitle("Reclaiming in One Call");

    stack.reset();
    std::cout << "used after reset(): " << stack.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
