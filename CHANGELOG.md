# Changelog

All notable changes to StackAllocator are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Nothing yet.

## [1.0.0] - 2026-07-30

The first stable release of StackAllocator, a fixed-capacity, bump-pointer
stack allocator for modern C++20.

### Added
- `Stack` allocator with O(1) bump-pointer `allocate()` over a single
  fixed-size buffer acquired at construction. Exhaustion returns `nullptr`
  rather than throwing or reallocating.
- `create<T>()` and `destroy()` for in-place construction and destruction of
  arbitrary types. `destroy()` runs `~T()` without reclaiming storage.
- `StackScope` RAII scope guard for automatic, exception-safe rollback.
- Marker-based checkpoints via `getMarker()` / `freeToMarker()` for LIFO
  rollback of allocations, with no fixed nesting-depth limit.
- `reset()` for O(1) bulk reclamation of the entire buffer.
- Alignment-aware allocation, including over-aligned types, bounded by a
  per-instance alignment ceiling set at construction.
- `owns()` for validating pointer provenance against a specific allocator
  instance.
- `used()`, `remaining()`, and `capacity()` introspection.
- Optional, zero-cost-when-disabled allocation statistics (total bytes
  allocated, current and peak usage, allocation count) via the compile-time
  `EnableStats` flag and `getStats()`.
- Move construction and move assignment; a moved-from `Stack` is left empty
  and reusable.
- Contract macros (`AP_PRE`, `AP_POST`, `AP_INVARIANT`, `AP_ASSERT`) mapping to
  `assert()` by default, replaceable globally, and expanding to no-ops when
  `NDEBUG` is defined. Preconditions are documented consistently across the
  API.
- Exception safety: allocation and constructor failures propagate without
  corrupting stack state.
- `rain::` umbrella namespace exposing the `StackPro` types.

### Performance
- A single upfront buffer allocation eliminates per-allocation heap traffic.
- Bump-pointer allocation keeps `allocate()` / `create()` to a handful of
  arithmetic operations.
- Marker-based rollback and `reset()` reclaim any number of allocations in
  O(1), without touching each one individually.
- Exhaustion is a plain bounds check and a `nullptr` return. The check is
  subtraction-based, so sizes close to `SIZE_MAX` cannot overflow it.
- Alignment is resolved via bit-shift arithmetic rather than a general
  modulo/division path.
- `owns()` is a pure range check against a single base pointer and capacity,
  with no per-allocation bookkeeping to search.
- `[[no_unique_address]]` statistics storage means a plain `Stack` (statistics
  disabled) carries zero bytes of bookkeeping for the statistics system.
- Benchmarked against `stdStack` (a naive, dynamically-growing baseline) at
  10K / 100K / 1M iterations; largest wins on the exhaustion path, large and
  over-aligned allocations, and `reset()` + refill churn. The one consistent
  loss is `Construction`, since the buffer is allocated eagerly and aligned up
  front. Full results in `benchmarks/results/v1_0_0.md`.

### Testing
- Comprehensive test suite covering unit, integration, lifecycle, and
  regression tests; move semantics; exception safety and allocation failure
  handling; marker-based rollback, including nested checkpoints; `StackScope`
  RAII behavior; alignment behavior; ownership validation (`owns()`);
  statistics tracking; and external synchronization contracts.
- 100.0% line coverage (93/93 lines) and 100.0% function coverage (47/47
  functions), excluding test infrastructure and third-party dependencies.

### CI
- Automated builds and tests across GCC, Clang, MSVC, and AppleClang, each
  in Debug and Release configurations.

[Unreleased]: https://github.com/privateMwb/StackAllocator/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/privateMwb/StackAllocator/releases/tag/v1.0.0
