# Test Suite

This document describes the test categories under `suite/` — what each
one verifies, and the individual test files it contains.

Unlike the benchmark suite, tests validate the library's own
correctness directly — there is no reference implementation to compare
against, so results are simply pass or fail.

Every test suite registers itself automatically via
`REGISTER_TEST_SUITE()` at startup, and is assigned a sequential id
within its category (e.g. `C1`, `C2` for Concurrency; `U1`, `U2` for
Unit) — there's no suite list to maintain by hand. This applies
uniformly across every category below.

---

## Concurrency

Verifies thread-safety — concurrent reads and writes from multiple
threads, and correctness under simultaneous access.

### Tests

- `external_locking_contract.cpp` — Concurrent allocate() calls and freeToMarker() rollbacks, serialized by a caller-supplied mutex, never overlap and total correctly
- `concurrent_read_only.cpp` — The const observers (used(), remaining(), capacity(), owns(), getStats()) stay consistent across concurrent, lock-free callers once state is settled
- `scope_per_thread.cpp` — One Stack/StackScope pair per thread, confirming independently-owned stacks don't leak state into one another

---

## Integration

Verifies multiple components working together end-to-end — for
example, Stack and StackScope combined across a realistic sequence —
rather than a single function in isolation.

### Tests

- `scope_rollback.cpp` — StackScope opens/closes via RAII, rolls back on a thrown exception, and freed bytes are reused
- `nested_scopes.cpp` — Nested scopes restore the correct marker at each level; rolling back an inner scope reuses its space while an outer scope's allocations are untouched
- `stats_tracking.cpp` — Stats stay correct across a mixed allocate/scope-rollback sequence; a rollback lowers currentUsed_ but never totalAllocated_ or allocations_
- `alloc_reset_reuse.cpp` — Fill to capacity, then reset(), then reuse from the start of the buffer
- `create_destroy_cycle.cpp` — destroy() alone doesn't reclaim storage; freeToMarker()/reset() does, and a later create() reuses the freed bytes

---

## Lifecycle

Verifies object lifetime operations — construction, destruction, and
moving.

### Tests

- `construction.cpp` — A fresh stack starts empty with full capacity; capacity() matches the requested size; stats start at zero when enabled
- `destruction.cpp` — A moved-from stack destructs safely; repeated construct/destroy cycles are stable; live, un-destroyed objects don't stop the buffer from being released
- `move_semantics.cpp` — Move construction and move assignment: state transfer, moved-from validity, self-move-assignment safety

---

## Regression

Verifies that a specific, previously fixed bug — or a deliberately
assert-only contract — stays exactly as intended. One test per
resolved issue or pinned contract, added at the time it's settled.

### Tests

- `overflow_guard.cpp` — allocate() rejects a size that would overflow the old addition-based bounds check instead of wrapping around and succeeding
- `alignment_contract.cpp` — requesting an alignment above the stack's construction alignment is an AP_PRE-only contract violation by design; also pins the fix for a moved-from stack falsely tripping that same precondition on its next default-alignment allocate() call

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

- `allocate.cpp` — Returns a valid pointer within capacity, nullptr when out of space, cursor advances across calls, honors requested alignment, rejects an oversized request cleanly
- `create.cpp` — Forwards constructor arguments, returns nullptr without constructing on a failed allocation, a throwing constructor propagates, storage is aligned to alignof(T)
- `destroy.cpp` — Runs the object's destructor without reclaiming its storage; nullptr is a no-op
- `get_marker.cpp` — A marker taken on a fresh stack is zero, reflects the current offset, and a later marker reads higher than an earlier one
- `free_to_marker.cpp` — Rewinds used() to the marker's checkpoint, freed space is reusable, freeing to the current marker is a no-op
- `reset.cpp` — Resets offset to zero, clears stats when enabled, leaves capacity() unchanged
- `owns.cpp` — Live allocations are owned; foreign pointers, the one-past-the-end address, and nullptr are not
- `alignment.cpp` — Default alignment matches alignof(max_align_t); a custom construction alignment is honored; padding consumes capacity as needed
- `observers.cpp` — used(), remaining(), capacity(), and getStats() all reflect current state correctly
