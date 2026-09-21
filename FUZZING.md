# Fuzzing

StackAllocator is fuzzed via [ClusterFuzzLite](https://google.github.io/clusterfuzzlite/),
running on every pull request that touches the fuzzed files, plus a
longer scheduled batch run every night.

## What's covered

**`fuzz_stack.cpp`** is a model-based fuzzer for `StackPro::Stack` (with
statistics both disabled and enabled) and `StackPro::StackScope`. It runs
a fuzzer-chosen sequence of operations against the real allocator and an
independent shadow model, and checks them against each other after every
single operation (not just at the end), so a failing input localizes to
the exact operation that broke an invariant.

The model tracks the expected bump offset, every live allocation (each
stamped with its own fill pattern), every outstanding `Marker`, and the
expected `Stats`. After each operation it verifies that:

- `allocate()`/`create()` succeed exactly when the model says there is
  room, and return exactly `base + alignedOffset`, correctly aligned and
  reported by `owns()`.
- No live allocation's fill pattern was clobbered — i.e. nothing
  overlaps, and rollback/reset never touches memory that is still live.
- `used()`, `remaining()`, `capacity()`, `getMarker()` and `getStats()`
  all agree with the model.

Specifically exercised:

- **Capacity boundaries.** Exact-fit allocations, one byte over, and
  sizes at/near `SIZE_MAX`. The subtraction-based bounds check in
  `allocate()` exists to avoid overflow there, and this is what
  stresses it.
- **Alignment padding.** Every per-call alignment from 1 up to the
  buffer's own alignment, across buffer alignments of 1 to 128 bytes.
  Capacities range from a handful of bytes (constantly exhausted) up to
  about 2 KiB.
- **`getMarker()`/`freeToMarker()`**, including markers that a later
  rollback or `reset()` has made stale. The harness never passes a stale
  marker back in (that would violate the documented precondition) but
  keeps using every marker that is still valid.
- **`StackScope`**, including nested scopes.
- **`reset()`**, including that it clears statistics.
- **`create<T>()`/`destroy<T>()`** across differently-aligned types
  (alignments of 1 to 64 bytes), plus a type with a non-trivial destructor to confirm
  `create()` constructs only on success and `destroy()` runs `~T()`
  exactly once. `destroy(nullptr)` is checked to be a no-op.
- **Move construction and move assignment, including self-move.** The
  moved-from stack must be left empty and reusable, and the buffer,
  offset and statistics must transfer intact.

Built and run under both AddressSanitizer and UndefinedBehaviorSanitizer,
with `-UNDEBUG` so the `AP_PRE`/`AP_POST`/`AP_INVARIANT`/`AP_ASSERT`
contract macros stay live. The harness itself never breaks a documented
precondition on purpose, so a contract failure it hits is a real finding.

## What's deliberately NOT covered yet

- **Constructor failure paths.** Capacities are capped at about 2 KiB,
  so the `std::bad_alloc` path in `Stack`'s constructor is never taken.
- **A `T` whose constructor throws inside `create()`.** The header
  documents that the reserved bytes are simply not reclaimed in that
  case; nothing here injects a throwing constructor to confirm it.
- **Deliberate precondition violations.** Zero-size allocations,
  non-power-of-two alignments, alignments above the buffer's, and
  stale markers are all contract violations and are never generated.
- **Thread safety.** `Stack` makes no thread-safety guarantees, so
  nothing here runs it concurrently.

## Running locally

```bash
git clone --recursive https://github.com/google/oss-fuzz.git
cd oss-fuzz
python infra/helper.py build_fuzzers --sanitizer address StackAllocator /path/to/StackAllocator
python infra/helper.py run_fuzzer StackAllocator fuzz_stack
```

Or, without OSS-Fuzz's tooling, directly with clang:

```bash
clang++ -std=c++20 -UNDEBUG -fsanitize=fuzzer,address \
  -Iinclude \
  fuzz/fuzz_stack.cpp \
  -o fuzz_stack

./fuzz_stack
```

Add `-fsanitize=fuzzer,undefined` instead to run under UBSan.

## Reproducing a crash

ClusterFuzzLite uploads the failing input as a workflow artifact when
a run fails. Download it, then:

```bash
./fuzz_stack path/to/crash-<hash>
```

This replays that exact byte sequence through
`LLVMFuzzerTestOneInput()` once, deterministically — no sanitizer flags
needed beyond however the binary was already built.

## Adding a new harness

1. Add `fuzz/fuzz_<target>.cpp` with an `extern "C" int
   LLVMFuzzerTestOneInput(const uint8_t*, size_t)` entry point.
2. Add the matching compile + link block to `.clusterfuzzlite/build.sh`.
3. No workflow changes needed — `cflite_pr.yml`/`cflite_batch.yml`
   build and run every binary `build.sh` produces in `$OUT`.
