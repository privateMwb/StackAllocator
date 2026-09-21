# Example Suite

This document describes the example categories under `suite/` — what each
one demonstrates, and the individual example files it contains.

| Category | Focus |
|---|---|
| [Advanced](#advanced) | Move semantics, exception safety, nested marker checkpoints, and allocation-statistics tracking |
| [Integration](#integration) | Embedding the stack in a class, constructing non-trivial types, and validating pointer ownership at a subsystem boundary |
| [Misuse](#misuse) | Common mistakes and the undefined behavior or contract violations they lead to |
| [Patterns](#patterns) | Scoped temporary allocation, stats-driven capacity sizing, one stack per thread, and bulk object creation |
| [Quickstart](#quickstart) | Construction, raw and typed allocation, object lifetime, and marker-based rollback |

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse where
instructive (see [Misuse](#misuse)), so the reader sees both the correct
pattern and the mistake it guards against.

Every example file ends with `REGISTER_EXAMPLE_SUITE()`, which derives the
suite's category from its containing directory and assigns it a sequential id
within that category. This applies uniformly across every category below.

---

## Advanced

Demonstrates deeper mechanics of the library — move semantics, exception
safety, nested marker checkpoints, and the optional allocation-statistics
tracking.

### Examples

| File | What it covers |
|---|---|
| `move_semantics.cpp` | Move construction/assignment, and what's actually safe to call on a moved-from stack |
| `exception_safety.cpp` | `create<T>()` when a constructor throws; the stack's offset is unaffected and the reserved bytes stay leaked until a rollback |
| `nested_markers.cpp` | Stacking several `getMarker()` checkpoints, and rolling back the innermost without disturbing the outer ones |
| `stats_tracking.cpp` | `Stack<true>`'s `totalAllocated_`/`currentUsed_`/`peakUsed_`/`allocations_`, and `peakUsed_` surviving a rollback |

---

## Integration

Demonstrates interoperability with the rest of a codebase — embedding the
stack inside a larger class, constructing non-trivial types, and validating
pointer ownership at a subsystem boundary.

### Examples

| File | What it covers |
|---|---|
| `embedding_in_class.cpp` | Wrapping `Stack` as a private implementation detail behind a domain-specific API |
| `custom_types.cpp` | Forwarding constructor arguments through `create<T>()` for a multi-member type |
| `owns_check.cpp` | `owns()` to validate pointer provenance, and reject pointers from another stack or the heap |

---

## Misuse

Demonstrates common mistakes and the undefined behavior or contract
violations they lead to, alongside the correct pattern — including examples
shown but not executed, so the reader can see what to avoid without the
program actually invoking undefined behavior.

### Examples

| File | What it covers |
|---|---|
| `stale_marker.cpp` | Reusing an outdated marker that's still technically valid but no longer means what the caller thinks |
| `out_of_space.cpp` | `allocate()` returning `nullptr` instead of throwing when capacity runs out |
| `dangling_after_free.cpp` | A pointer left dangling by a `freeToMarker()` rollback |
| `misaligned_request.cpp` | Exceeding the buffer's construction alignment (shown, not executed) |

---

## Patterns

Demonstrates common usage idioms built on top of the core API — scoped
temporary allocation, stats-driven capacity sizing, one stack per thread,
and bulk object creation.

### Examples

| File | What it covers |
|---|---|
| `scoped_temp_alloc.cpp` | `StackScope` for short-lived working memory inside a function |
| `stats_driven_sizing.cpp` | Using `peakUsed_` from a representative workload to size a production stack |
| `stack_per_thread.cpp` | One stack per thread instead of synchronizing access to a shared one |
| `bulk_struct_alloc.cpp` | `create<T>()` in a loop until the stack runs out, and detecting why it stopped |

---

## Quickstart

Demonstrates fundamental, everyday usage — construction, raw and typed
allocation, object lifetime, and marker-based rollback.

### Examples

| File | What it covers |
|---|---|
| `basic_usage.cpp` | Construction, `allocate()`, `create<T>()`, `destroy()`, `capacity`/`used`/`remaining`, `reset()` |
| `create_destroy.cpp` | Object lifetime across several `create<T>()` calls, and why `destroy()` doesn't shrink `used()` |
| `marker_rollback.cpp` | `getMarker()`/`freeToMarker()` as a basic checkpoint/restore pair |

---
