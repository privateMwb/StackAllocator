# Benchmark Suite

This document describes the benchmark categories under `benchmarks/` — what each
one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Access](#access) | Read-only ownership checks and state queries on an already-populated stack |
| [Core](#core) | Allocating, constructing, destroying, resetting, and marker rollback |
| [Lifecycle](#lifecycle) | Construction, moving, and scoped rollback |
| [Scaling](#scaling) | Cost vs. capacity, alignment, and exhaustion, independent of iteration count |
| [Utility](#utility) | Running allocation statistics and the `EnableStats` toggle |
| [Conventions](#conventions) | Registration, sizing, and elision conventions specific to the custom framework |

Every benchmark compares StackPro's `Stack` against `stdStack` — a
`std::pmr::monotonic_buffer_resource`, the standard library's own linear/bump
allocator, and the conventional way this kind of allocation behavior is built
in C++. A category can support more than one standard for comparison, but for
now each category is benchmarked against a single standard.

Every `BENCH()` call, in every category below, is automatically repeated at
three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) — to
smooth out timing noise and show whether relative performance holds steady
as call volume increases. This applies uniformly across the whole suite; it
is not specific to any one category. The **Scaling** category below measures
something different: how per-operation cost changes as capacity itself
grows or shrinks, independent of iteration count.

Some benchmarks have no meaningful stdStack equivalent — a bare
`memory_resource` tracks no ownership, usage, or allocation statistics,
supports no nested checkpoints or scoped rollback, and isn't movable. Those
run through `BENCH_SOLO()` instead of `BENCH()`, timing Stack alone.

---

## Access

Benchmarks read-only operations against a stack that is already holding
allocations — ownership checks and querying current usage.

### Benchmarks

| File | What it covers |
|---|---|
| `ownership.cpp` | `owns()` hit and `owns()` miss (solo, no stdStack equivalent) |
| `state_query.cpp` | `used()`, `remaining()`, and `capacity()` (solo, no stdStack equivalent) |

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
allocating raw bytes, constructing in place, destroying, and reclaiming
space via reset or marker rollback.

### Benchmarks

| File | What it covers |
|---|---|
| `allocate.cpp` | `allocate()` small, large, and over-aligned |
| `construct.cpp` | `create<T>()` with a trivial constructor, and with a non-trivial multi-argument constructor |
| `destroy.cpp` | `destroy<T>()` with a trivial destructor, and with a non-trivial destructor |
| `reset_refill.cpp` | `reset()` alone, and `reset()` then refilling to a fixed entry count, against stdStack's `release()` |
| `marker_rollback.cpp` | `getMarker()` alone, `freeToMarker()` alone, and the get+free round trip (solo, no stdStack equivalent) |
| `nested_markers.cpp` | Building and unwinding a depth-8 chain of nested markers (solo, no stdStack equivalent) |

---

## Lifecycle

Benchmarks object lifetime operations — construction, moving, and scoped
rollback via `StackScope`.

### Benchmarks

| File | What it covers |
|---|---|
| `construction.cpp` | Constructing an empty stack sized for N bytes — Stack allocates eagerly, stdStack defers to first use, so this compares two genuinely different construction strategies |
| `move.cpp` | Move construction and move assignment (solo — `monotonic_buffer_resource` is neither copyable nor movable) |
| `scope_raii.cpp` | `StackScope` construction/destruction (solo, no stdStack equivalent) |

---

## Scaling

Benchmarks how per-operation cost changes as capacity itself grows or
shrinks — a separate axis from the SMALL/MEDIUM/LARGE iteration tiers
described above: those repeat the same fixed-size operation more times,
while Scaling grows or shrinks the buffer, alignment, or remaining headroom
itself and observes the resulting cost.

### Benchmarks

| File | What it covers |
|---|---|
| `capacity_growth.cpp` | `allocate()` across increasing buffer sizes: 4 KiB, 1 MiB, and 64 MiB |
| `alignment_scaling.cpp` | `allocate()` across increasing alignment requests: 4, 64, and 4096 bytes |
| `exhaustion.cpp` | `allocate()` with room to spare, and `allocate()` at capacity (failure path), against a bounded stdStack using `std::pmr::null_memory_resource()` as its upstream |

---

## Utility

Benchmarks bookkeeping operations that don't belong to any of the categories
above — running allocation statistics and the cost of the `EnableStats`
toggle.

### Benchmarks

| File | What it covers |
|---|---|
| `stats.cpp` | `getStats()` on a stack with outstanding allocations (solo, no stdStack equivalent) |
| `stats_toggle.cpp` | `allocate()` with `EnableStats` disabled versus enabled (solo, no runtime equivalent to pair against) |

---

## Conventions

- **Registration** — every case is a `static void bench_<name>()` called from
  the file's `run_benchmarks()`, and the file ends with
  `REGISTER_BENCH_SUITE();`. `BENCH("label", c, s)` runs Stack (`c`) against
  stdStack (`s`); `BENCH_SOLO("label", a)` times Stack alone and is used only
  when stdStack has no equivalent.
- **Preventing elision** — every lambda passes its result to
  `doNotOptimize()` so the call can't be optimized away. A lambda with no
  result (`reset()`, `destroy()`, `freeToMarker()`) has nothing to pass.
- **Matching work** — the stdStack side must do the same work as the Stack
  side: `create<T>()` pairs with `std::pmr::polymorphic_allocator<T>`
  `allocate(1)` plus `construct()`, `destroy<T>()` with the same allocator's
  `destroy()`, and `reset()` with `release()`.
- **Buffer sizing** — every `BENCH()` repeats at the SMALL, MEDIUM, and LARGE
  tiers, so the buffer is sized generously above the LARGE tier and never
  exhausts mid-run. Where that isn't practical (small or swept buffer sizes,
  large alignments), each lambda resets first — `reset()` for Stack,
  `release()` for stdStack — to roll the cursor back on every call.
- **Consumed inputs** — setup stays outside the lambda, except for operations
  that need fresh state every call. `nested_markers.cpp` resets the stack and
  clears its pre-reserved marker vector at the top of every call, and
  `destroy.cpp` reserves one fixed block outside the lambda, then
  placement-constructs and destroys into it on each call, since destroying an
  object twice is undefined behavior. `move.cpp` builds its source once, so
  after the first call it moves an already-empty stack, which is safe because
  Stack's move operations don't branch on a null source.
