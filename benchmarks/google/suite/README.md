# Google Benchmark Suite

This document describes the benchmark categories under `benchmarks/` — what each
one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Access](#access) | Read-only ownership checks and state queries on an already-populated stack |
| [Core](#core) | Allocating, constructing, destroying, resetting, and marker rollback |
| [Lifecycle](#lifecycle) | Construction, moving, and scoped rollback |
| [Scaling](#scaling) | Cost vs. capacity, alignment, and exhaustion, independent of iteration count |
| [Utility](#utility) | Running allocation statistics and the `EnableStats` toggle |
| [Conventions](#conventions) | Registration, sizing, and elision conventions specific to Google Benchmark |

Every benchmark compares StackPro's `Stack` against `stdStack` — a
`std::pmr::monotonic_buffer_resource`, the standard library's own linear/bump
allocator, and the conventional way this kind of allocation behavior is built
in C++. A category can support more than one standard for comparison, but for
now each category is benchmarked against a single standard.

Every benchmark is run by [Google Benchmark](https://github.com/google/benchmark),
which scales each one's iteration count automatically until timing is stable —
there are no fixed iteration tiers. A handful of benchmarks that never reset
their buffer pin a fixed `->Iterations(N)` instead, so they can't exhaust it
mid-run (see [Conventions](#conventions)). This applies uniformly across the
whole suite; it is not specific to any one category. The **Scaling** category
below measures something different: how per-operation cost changes as capacity
itself grows or shrinks, independent of iteration count.

Where a comparison exists, it is two benchmarks — `<group>_stack` and
`<group>_std` — reported as two rows of the same table. Some benchmarks have no
meaningful stdStack equivalent — a bare `memory_resource` tracks no ownership,
usage, or allocation statistics, supports no nested checkpoints or scoped
rollback, and isn't movable. Those are solo: a single benchmark timing `Stack`
alone, with no `_std` counterpart.

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
shrinks — a separate axis from iteration count: iteration count repeats the
same fixed-size operation more times, while Scaling grows or shrinks the
buffer, alignment, or remaining headroom itself and observes the resulting
cost.

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

- **Registration** — every case is a
  `static void <group>_<variant>(benchmark::State& state)` that does its work
  in a `for (auto _ : state)` loop, registered directly below it with
  `BENCHMARK(<group>_<variant>)`. The function name is the benchmark name — no
  `->Name()` override, no `BM_` prefix, and no `BENCHMARK_MAIN()` (`main` is
  supplied separately). Everything before the final `_<variant>` is the
  group, and benchmarks that share a group are reported in the same table.
  A paired case uses the variants `_stack` and `_std`
  (`allocate_small_stack` / `allocate_small_std`); related solo cases share a
  group through their own variants (`owns_hit` / `owns_miss`,
  `move_construct` / `move_assign`), and are used only when stdStack has no
  equivalent.
- **Preventing elision** — every loop passes its result to
  `benchmark::DoNotOptimize()` so the call can't be optimized away. A loop
  with no result (`reset()`, `destroy()`, `freeToMarker()`) has nothing to
  pass.
- **Matching work** — the stdStack side must do the same work as the Stack
  side: `create<T>()` pairs with `std::pmr::polymorphic_allocator<T>`
  `allocate(1)` plus `construct()`, `destroy<T>()` with the same allocator's
  `destroy()`, and `reset()` with `release()`.
- **Buffer sizing** — Google Benchmark chooses iteration counts on its own, so
  any benchmark that never resets its buffer pins `->Iterations(N)` to a count
  whose worst-case consumption stays well under capacity (`allocate.cpp`,
  `construct.cpp`, the headroom cases in `exhaustion.cpp`, and
  `stats_toggle.cpp`). Where a small buffer couldn't absorb that many calls
  (`capacity_growth.cpp`, `alignment_scaling.cpp`), every iteration resets
  first — `reset()` for Stack, `release()` for stdStack — bounding its
  footprint to a single allocation.
- **Consumed inputs** — setup stays outside the loop, except for operations
  that need fresh state every iteration. `nested_markers.cpp` resets the stack
  and clears its pre-reserved marker vector at the top of every iteration, and
  `destroy.cpp` reserves one fixed block outside the loop, then
  placement-constructs and destroys into it on each iteration, since destroying
  an object twice is undefined behavior. `move.cpp` builds its source once, so
  after the first iteration it moves an already-empty stack, which is safe
  because Stack's move operations don't branch on a null source.
