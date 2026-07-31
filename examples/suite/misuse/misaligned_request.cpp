// Requesting more alignment than the buffer was built with.
//
// Demonstrates:
// - The precondition that per-call alignment can't exceed the
//   alignment chosen at construction
// - Why this is an AP_PRE (assert), not a checked runtime failure —
//   violating it is undefined behavior in release builds
// - The fix: size the buffer's alignment for the strictest type
//   it will ever hold

#include <support/framework.h>

using namespace StackPro;

static void run_examples() {

    // The alignment given at construction is a ceiling, not a default —
    // every later allocate()/create() must fit under it.
    setTitle("Buffer Alignment Is a Ceiling");

    Stack<> under_aligned(1024); // defaults to alignof(std::max_align_t), typically 16

    std::byte* ok = under_aligned.allocate(64, 16);
    std::cout << "allocate(64, 16) on a 16-byte-aligned stack: " << (ok != nullptr) << "\n\n";

    // Requesting 32-byte alignment from a stack only built for 16 is a
    // precondition violation. It's checked via AP_PRE in debug builds
    // (an assert that aborts), and simply undefined in release builds —
    // there's no nullptr return to catch this like there is for running
    // out of capacity. The call is left commented out on purpose: this
    // would either abort (debug) or hand back an under-aligned block
    // that silently miscompiles SIMD loads (release).
    setTitle("Exceeding It Is Undefined Behavior, Not a Failure");

    std::cout << "// under_aligned.allocate(64, 32); <- AP_PRE violation\n\n";

    // The fix is to build the stack with enough alignment up front for
    // whatever it will ever be asked to hand out.
    setTitle("Fixing It: Align the Buffer Up Front");

    Stack<> simd_ready(1024, 32);

    std::byte* aligned_block = simd_ready.allocate(64, 32);
    std::cout << "allocate(64, 32) on a 32-byte-aligned stack: " << (aligned_block != nullptr)
              << "\n";
    std::cout << "address is 32-byte aligned: "
              << (reinterpret_cast<std::uintptr_t>(aligned_block) % 32 == 0) << "\n";
}

REGISTER_EXAMPLE_SUITE();
