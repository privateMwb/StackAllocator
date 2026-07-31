# Example Suite

This document describes the example categories under `suite/` — what
each one demonstrates, and the individual example files it contains.

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse
where instructive (see Misuse), so the reader sees both the correct
pattern and the mistake it guards against.

Every example file ends with `REGISTER_EXAMPLE_SUITE()`, which derives
the suite's category from its containing directory and assigns it a
sequential id within that category. This applies uniformly across
every category below.

---

## Advanced

Demonstrates deeper mechanics of the library — move semantics,
exception safety, nested marker checkpoints, and the optional
allocation-statistics tracking.

### Examples

- `move_semantics.cpp` — move construction/assignment, and what's actually safe to call on a moved-from stack
- `exception_safety.cpp` — create<T>() when a constructor throws; the stack's offset is unaffected and the reserved bytes stay leaked until a rollback
- `nested_markers.cpp` — stacking several getMarker() checkpoints, and rolling back the innermost without disturbing the outer ones
- `stats_tracking.cpp` — Stack<true>'s totalAllocated_/currentUsed_/peakUsed_/allocations_, and peakUsed_ surviving a rollback

---

## Integration

Demonstrates interoperability with the rest of a codebase — embedding
the stack inside a larger class, constructing non-trivial types, and
validating pointer ownership at a subsystem boundary.

### Examples

- `embedding_in_class.cpp` — wrapping Stack as a private implementation detail behind a domain-specific API
- `custom_types.cpp` — forwarding constructor arguments through create<T>() for a multi-member type
- `owns_check.cpp` — owns() to validate pointer provenance, and reject pointers from another stack or the heap

---

## Misuse

Demonstrates common mistakes and the undefined behavior or contract
violations they lead to, alongside the correct pattern — including
examples shown but not executed, so the reader can see what to avoid
without the program actually invoking undefined behavior.

### Examples

- `stale_marker.cpp` — reusing an outdated marker that's still technically valid but no longer means what the caller thinks
- `out_of_space.cpp` — allocate() returning nullptr instead of throwing when capacity runs out
- `dangling_after_free.cpp` — a pointer left dangling by a freeToMarker() rollback
- `misaligned_request.cpp` — exceeding the buffer's construction alignment (shown, not executed)

---

## Patterns

Demonstrates common usage idioms built on top of the core API —
scoped temporary allocation, stats-driven capacity sizing, one stack
per thread, and bulk object creation.

### Examples

- `scoped_temp_alloc.cpp` — StackScope for short-lived working memory inside a function
- `stats_driven_sizing.cpp` — using peakUsed_ from a representative workload to size a production stack
- `stack_per_thread.cpp` — one stack per thread instead of synchronizing access to a shared one
- `bulk_struct_alloc.cpp` — create<T>() in a loop until the stack runs out, and detecting why it stopped

---

## Quickstart

Demonstrates fundamental, everyday usage — construction, raw and
typed allocation, object lifetime, and marker-based rollback.

### Examples

- `basic_usage.cpp` — construction, allocate(), create<T>(), destroy(), capacity/used/remaining, reset()
- `create_destroy.cpp` — object lifetime across several create<T>() calls, and why destroy() doesn't shrink used()
- `marker_rollback.cpp` — getMarker()/freeToMarker() as a basic checkpoint/restore pair
