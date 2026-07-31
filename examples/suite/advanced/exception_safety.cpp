// Exception safety of create<T>().
//
// Demonstrates:
// - create<T>() reserving bytes before running T's constructor
// - What happens to the stack's state if that constructor throws
// - Why the reserved bytes stay leaked until a rollback

#include <support/framework.h>

using namespace StackPro;

namespace {

struct Fussy {
    explicit Fussy(bool should_throw) {
        if (should_throw)
            throw std::runtime_error("Fussy refused to construct");
    }
};

} // namespace

static void run_examples() {

    // allocate() runs first and bumps the offset — construction happens
    // afterward, in the already-reserved bytes.
    setTitle("Before the Throw");

    Stack<> stack(1024);
    Stack<>::Marker start = stack.getMarker();

    std::cout << "used before create<T>(): " << stack.used() << "\n\n";

    // If T's constructor throws, create<T>() propagates the exception —
    // but the bytes it reserved were already committed to offset_ and
    // are not given back.
    setTitle("Constructor Throws");

    try {
        (void)stack.create<Fussy>(true);
    } catch (const std::exception& e) {
        std::cout << "caught: " << e.what() << "\n";
    }

    std::cout << "used after the throw (leaked): " << stack.used() << "\n\n";

    // A successful create<T>() right after leaves the stack in a normal,
    // fully consistent state — the earlier leak doesn't corrupt anything,
    // it's just unreclaimed space.
    setTitle("Stack Remains Usable");

    Fussy* ok = stack.create<Fussy>(false);
    std::cout << "second create<T>() succeeded: " << (ok != nullptr) << "\n\n";

    // The only way to get the leaked bytes back is a rollback — reset()
    // or freeToMarker() to a point before the throwing call.
    setTitle("Reclaiming the Leak");

    stack.destroy(ok);
    stack.freeToMarker(start);

    std::cout << "used after freeToMarker(): " << stack.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
