// Constructing non-trivial types with create<T>().
//
// Demonstrates:
// - Perfect-forwarding constructor arguments through create<T>()
// - Types with multiple members and non-trivial constructors
// - Types that own heap memory of their own (a std::string member)

#include <support/framework.h>

#include <string>
#include <vector>

using namespace StackPro;

namespace {

// A type with several members and a constructor that does real work —
// not just an aggregate.
struct Particle {
    std::string name;
    std::vector<float> velocity;
    int health;

    Particle(std::string name, std::vector<float> velocity, int health)
        : name{std::move(name)}, velocity{std::move(velocity)}, health{health} {}
};

// A type with no default constructor, to confirm create<T>() doesn't
// require one — only that some constructor matches the forwarded args.
struct NonDefault {
    int value;
    explicit NonDefault(int value) : value{value} {}
    NonDefault() = delete;
};

} // namespace

static void run_examples() {

    // Arguments are forwarded straight to Particle's constructor — moved,
    // not copied, where the constructor itself takes them by value/move.
    setTitle("Forwarding Constructor Arguments");

    Stack<> stack(2048);

    Particle* p = stack.create<Particle>("drone", std::vector<float>{1.0f, 0.5f, 0.0f}, 100);

    std::cout << "name    : " << p->name << "\n";
    std::cout << "velocity: [" << p->velocity[0] << ", " << p->velocity[1] << ", " << p->velocity[2]
              << "]\n";
    std::cout << "health  : " << p->health << "\n\n";

    // The Particle itself lives in the stack's buffer, but its string and
    // vector members still own their own heap allocations — the stack
    // only owns the bytes for the Particle object, not what it points to.
    setTitle("Members Still Own Their Own Memory");

    stack.destroy(p);
    std::cout << "destroy() ran ~Particle(), which frees name/velocity's heap storage\n\n";

    // create<T>() works fine for types with no default constructor, since
    // it only needs a constructor matching the arguments given.
    setTitle("Types Without a Default Constructor");

    NonDefault* nd = stack.create<NonDefault>(42);
    std::cout << "value: " << nd->value << "\n";
}

REGISTER_EXAMPLE_SUITE();
