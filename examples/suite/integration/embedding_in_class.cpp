// Embedding a Stack inside another class.
//
// Demonstrates:
// - Keeping a Stack as a private implementation detail
// - Exposing a narrow, purpose-built interface instead of the raw API
// - Tying reset() to the owning class's own lifecycle

#include <support/framework.h>

using namespace StackPro;

namespace {

// A tiny per-frame scratch buffer for a rendering-style system. Callers
// never see the Stack directly — only what FrameScratch chooses to expose.
class FrameScratch {
  public:
    explicit FrameScratch(size_t bytes) : stack_(bytes) {}

    template <typename T, typename... Args> T* push(Args&&... args) {
        return stack_.create<T>(std::forward<Args>(args)...);
    }

    size_t bytesUsed() const {
        return stack_.used();
    }

    // The class decides when scratch memory is recycled — callers don't
    // get to reach in and call reset() themselves.
    void beginFrame() {
        stack_.reset();
    }

  private:
    Stack<> stack_;
};

struct Vertex {
    float x, y, z;
};

} // namespace

static void run_examples() {

    // The Stack lives entirely inside FrameScratch — construction is the
    // only place its size is chosen.
    setTitle("Wrapping Stack in a Class");

    FrameScratch scratch(4096);
    std::cout << "bytes used at start: " << scratch.bytesUsed() << "\n\n";

    // The wrapper's push<T>() forwards straight to create<T>(), so callers
    // still get typed, constructed objects — just through a narrower door.
    setTitle("Using the Narrow Interface");

    Vertex* v1 = scratch.push<Vertex>(0.0f, 0.0f, 0.0f);
    Vertex* v2 = scratch.push<Vertex>(1.0f, 0.0f, 0.0f);
    (void)v1;
    (void)v2;

    std::cout << "bytes used after 2 vertices: " << scratch.bytesUsed() << "\n\n";

    // The owning class controls the reset policy — here, once per frame —
    // instead of leaving it to whoever holds a reference.
    setTitle("Lifecycle-Driven Reset");

    scratch.beginFrame();
    std::cout << "bytes used after beginFrame(): " << scratch.bytesUsed() << "\n";
}

REGISTER_EXAMPLE_SUITE();
