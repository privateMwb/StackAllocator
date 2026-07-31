# Benchmark Suite

This document describes the benchmark categories under `benchmarks/` —
what each one measures, and the individual benchmark files it
contains.

Every benchmark is timed at three iteration tiers — SMALL (10K),
MEDIUM (100K), and LARGE (1M) — and reported against `stdStack`
(`std::pmr::monotonic_buffer_resource`) via `BENCH()` wherever a
meaningful standard equivalent exists. Where no equivalent exists, the
case runs alone via `BENCH_SOLO()` and is reported without a
comparison column.

Every benchmark suite registers itself automatically via
`REGISTER_BENCH_SUITE()` at startup, and is assigned a sequential id
within its category (e.g. `A1`, `A2` for Access; `S1`, `S2`, `S3` for
Scaling) — there's no suite list to maintain by hand. This applies
uniformly across every category below.

---

## Access

Measures read-only operations against an already-populated stack —
ownership checks and introspection accessors.

### Benchmarks

- `ownership.cpp` — owns() on a pointer that belongs to the stack (hit path) and one that doesn't (miss path) — solo, stdStack has no ownership query
- `state_query.cpp` — used(), remaining(), and capacity() on a partially-filled stack — solo, stdStack exposes none of these

---

## Core

Measures the allocator's fundamental hot-path operations: allocation,
construction, destruction, and checkpoint/rollback.

### Benchmarks

- `allocate.cpp` — allocate() of a small block, a larger block, and a small block at an over-aligned boundary — paired against stdStack
- `construct.cpp` — create<T>() with a trivial constructor and a non-trivial, multi-argument constructor — paired against stdStack via std::pmr::polymorphic_allocator<T>
- `destroy.cpp` — destroy<T>() with a trivial destructor and a non-trivial destructor, isolated from allocation cost — paired against stdStack via std::pmr::polymorphic_allocator<T>
- `reset_refill.cpp` — reset() alone, and reset() followed by refilling to a fixed entry count — paired against stdStack's release()
- `marker_rollback.cpp` — getMarker() alone, freeToMarker() alone, and the get+free round trip — solo, stdStack has no partial-rollback checkpoint
- `nested_markers.cpp` — building and unwinding a depth-8 chain of nested markers — solo, stdStack has no checkpoint concept at all

---

## Lifecycle

Measures object lifetime operations: construction, destruction,
moving, and scoped rollback.

### Benchmarks

- `construction.cpp` — constructing (and destroying) an empty allocator sized for a fixed byte count — paired against stdStack, though the two use different construction strategies (Stack allocates eagerly; stdStack may defer)
- `move.cpp` — move-construction and move-assignment — solo, stdStack is neither copyable nor movable
- `scope_raii.cpp` — StackScope construction immediately followed by destruction — solo, stdStack has no equivalent scoped-checkpoint type

---

## Scaling

Measures how allocate() behaves as its inputs and constraints grow:
capacity, alignment, and remaining headroom.

### Benchmarks

- `capacity_growth.cpp` — allocate() against a 4 KiB, 1 MiB, and 64 MiB buffer — paired against stdStack
- `alignment_scaling.cpp` — allocate() at 4-, 64-, and 4096-byte alignment — paired against stdStack
- `exhaustion.cpp` — allocate() with room to spare, and allocate() once completely full — paired against a bounded stdStack whose upstream throws on the failure path

---

## Utility

Measures the allocator's optional statistics tracking, and the cost of
the EnableStats compile-time toggle itself.

### Benchmarks

- `stats.cpp` — getStats() on a stack with outstanding allocations — solo, stdStack tracks no statistics of its own
- `stats_toggle.cpp` — allocate() with EnableStats disabled versus enabled, read side by side — solo, there is no runtime equivalent to pair against
