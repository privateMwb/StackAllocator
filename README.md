# StackPro

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![Status](https://img.shields.io/badge/status-learning%20project-green)](https://github.com/privateMwb/StackPro)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A custom C++ stack allocator implementation built for learning low-level memory management, linear allocation strategies, marker-based deallocation, and performance benchmarking.

---

## Table of Contents

- [Overview](#overview)
- [Motivation / Goals](#motivation--goals)
- [Features](#features)
- [Design Overview](#design-overview)
- [Complexity](#complexity)
- [Quick Example](#quick-example)
- [Core API](#core-api)
- [Benchmark Results](#benchmark-results)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Notes](#notes)
- [License](#license)

---

## Overview

StackPro (`Stack`) is a linear stack allocator implemented from scratch in modern C++ (C++23).
It focuses on understanding how stack allocators work internally, including bump pointer allocation, marker-based deallocation, aligned memory management, and RAII scope guards.

It also includes:

- Typed object construction and destruction via `create<T>()` / `destroy<T>()`
- Marker-based deallocation via `getMarker()` / `freeToMarker()`
- RAII scope guard via `StackScope`
- O(1) bulk reset via `reset()`
- Ownership queries via `owns()`
- Optional debug statistics via `Stack<true>`
- Benchmark suite comparing against heap (`new` / `delete`)
- Unit tests for correctness validation

---

## Motivation / Goals

This project was built to understand:

- Linear bump pointer allocation strategies
- Marker-based deallocation without per-object metadata
- Aligned memory management using `::operator new`
- Object lifecycle: construction and destruction within a stack
- RAII scope guard patterns for automatic stack unwinding
- Optional compile-time statistics with zero overhead when disabled
- Performance benchmarking vs heap allocation

---

## Features

- O(1) allocation via bump pointer
- O(1) deallocation via marker restoration
- Aligned allocation with configurable default and per-request alignment
- Typed object creation with forwarded constructor arguments via `create<T>()`
- Explicit destructor invocation via `destroy<T>()`
- RAII scope guard via `StackScope` for automatic marker restoration
- Full reset to reclaim all memory in O(1) without calling destructors
- Ownership query via `owns()`
- Optional debug statistics via `Stack<true>` with zero overhead when disabled
- `[[no_unique_address]]` on stats storage — no size penalty when stats are off
- Opaque `Marker` type — prevents raw integer misuse in `freeToMarker()`
- Move semantics with deleted copy
- `std::constructible_from` concept constraint on `create<T>()`

---

## Design Overview

Stack uses a single contiguous heap-allocated slab with a bump pointer for allocation and marker-based restoration for deallocation.

### Internal Structure

```
memory_ (pointer)
  ↓
[alloc 0][alloc 1][alloc 2][padding][alloc 3][...]
                                              ↑
                                           offset_
```

- `memory_`    → pointer to raw allocated slab
- `cap_`       → total capacity in bytes
- `offset_`    → current bump pointer position
- `alignShift_`→ log2 of the default alignment
- `stats_`     → optional debug statistics (zero-size when disabled)

### Allocation Strategy

Allocation aligns the current offset and bumps it forward:

```cpp
const std::size_t alignedOffset = alignForward(offset_, toShift(requestAlignment));
std::byte* ptr = memory_ + alignedOffset;
offset_        = alignedOffset + size;
return ptr;
```

No heap traffic after construction. No per-allocation metadata.

### Deallocation Strategy

Deallocation restores the offset to a previously captured marker:

```cpp
offset_ = marker.get();
```

All memory allocated after the marker is reclaimed in O(1).

### Marker System

`getMarker()` captures the current offset as an opaque `Marker`:

```cpp
auto marker = stack.getMarker();   // capture current position
// ... allocations ...
stack.freeToMarker(marker);        // restore to captured position
```

`Marker` is an opaque type — raw integers cannot be passed to `freeToMarker()`.

### StackScope

`StackScope` is a RAII guard that captures a marker on construction and calls `freeToMarker` on destruction:

```cpp
{
    StackScope scope{stack};
    // ... allocations automatically unwound on scope exit ...
}
```

Nested scopes unwind in LIFO order naturally.

### Alignment Strategy

Alignment is stored as a bit shift (`alignShift_`) rather than a raw value.
This turns alignment math into a bitmask operation with no division:

```cpp
const std::size_t mask = (std::size_t{1} << shift) - 1u;
return (ptr + mask) & ~mask;
```

### Object Lifecycle

`create<T>()` allocates aligned memory and placement-constructs the object:

```cpp
T* obj = stack.create<T>(args...);
```

`destroy<T>()` invokes the destructor without releasing memory:

```cpp
stack.destroy(obj);   // destructor called, memory remains in stack
```

### Optional Statistics

Statistics are controlled at compile time via the `EnableStats` template parameter:

```cpp
Stack<false> stack{1024};        // no stats — zero overhead
Stack<true>  debug{1024};        // stats enabled
```

`[[no_unique_address]]` ensures the stats struct occupies zero bytes when disabled.

### Exception Safety Model

- `allocate()` returns `nullptr` on exhaustion — no exceptions
- `create<T>()` returns `nullptr` if allocation fails
- Move operations are `noexcept`
- `reset()`, `freeToMarker()`, `destroy<T>()` are `noexcept`
- `freeToMarker()` is precondition-guarded — marker must be <= current offset

---

## Complexity

### Time Complexity

| Operation        | Complexity | Notes                                      |
| ---------------- | ---------- | ------------------------------------------ |
| `allocate`       | O(1)       | Bump pointer + alignment mask              |
| `freeToMarker`   | O(1)       | Offset restore                             |
| `create<T>`      | O(1)       | Allocation + placement construction        |
| `destroy<T>`     | O(1)       | Destructor invocation only                 |
| `reset`          | O(1)       | Offset reset to zero                       |
| `owns`           | O(1)       | Bounds check                               |
| `getMarker`      | O(1)       | Offset capture                             |
| `getStats`       | O(1)       | Reference return                           |

### Space Complexity

- O(n) for the backing slab (`size` bytes)
- O(1) for all metadata
- O(0) for stats when `EnableStats = false`

### Notes

- No per-allocation overhead — marker-based deallocation requires no metadata per block
- `reset()` does not call destructors — caller is responsible for object cleanup
- `freeToMarker()` reclaims all memory after the marker in a single assignment
- Alignment padding may consume bytes between allocations depending on request alignment

---

## Quick Example

### Basic Allocation

```cpp
#include "Stack.h"

using namespace AllocatorPro;

int main() {
    Stack s{1024};

    std::byte* a = s.allocate(128);
    std::byte* b = s.allocate(256);
    (void)a;
    (void)b;

    s.reset();   // reclaim all memory in O(1)
}
```

### Marker-Based Deallocation

```cpp
#include "Stack.h"

using namespace AllocatorPro;

int main() {
    Stack s{1024};

    (void)s.allocate(128);

    auto marker = s.getMarker();   // capture current position
    (void)s.allocate(256);

    s.freeToMarker(marker);        // restore to captured position
}
```

### RAII Scope Guard

```cpp
#include "Stack.h"
#include "StackScope.h"

using namespace AllocatorPro;

int main() {
    Stack s{1024};
    (void)s.allocate(128);

    {
        StackScope scope{s};
        (void)s.allocate(256);
    }   // automatically restored to pre-scope position
}
```

### Object Lifecycle

```cpp
#include "Stack.h"

using namespace AllocatorPro;

struct Particle {
    float x, y, z;
    Particle(float x, float y, float z) : x(x), y(y), z(z) {}
    ~Particle() {}
};

int main() {
    Stack s{1024};

    Particle* p = s.create<Particle>(1.0f, 2.0f, 3.0f);
    s.destroy(p);   // destructor called, memory remains in stack
    s.reset();      // reclaim all memory in O(1)
}
```

### Debug Statistics

```cpp
#include "Stack.h"

using namespace AllocatorPro;

int main() {
    Stack<true> s{1024};

    (void)s.allocate(128);
    (void)s.allocate(256);

    const auto& stats = s.getStats();
    // stats.allocations_, stats.totalAllocated_, stats.peakUsed_, stats.currentUsed_
}
```

---

## Core API

### Constructors

```cpp
Stack s{size};                        // default alignment
Stack s{size, alignment};             // custom alignment
Stack b{std::move(a)};               // move construction
b = std::move(a);                    // move assignment
```

### Memory Management

```cpp
[[nodiscard]] std::byte* allocate(std::size_t size,
    std::size_t request_alignment = alignof(std::max_align_t)) noexcept;

[[nodiscard]] Marker getMarker() const noexcept;
void freeToMarker(Marker marker) noexcept;
```

### Object Lifecycle

```cpp
template<typename T, typename... Args>
requires (!std::is_array_v<T>) && std::constructible_from<T, Args...>
[[nodiscard]] T* create(Args&&... args);

template<typename T>
requires (!std::is_array_v<T>)
void destroy(T* ptr) noexcept;
```

### State Management

```cpp
void reset() noexcept;
```

### Introspection

```cpp
[[nodiscard]] bool owns(const void* ptr) const noexcept;

[[nodiscard]] const Stats& getStats() const noexcept requires EnableStats;

[[nodiscard]] std::size_t used()      const noexcept;
[[nodiscard]] std::size_t remaining() const noexcept;
[[nodiscard]] std::size_t capacity()  const noexcept;
```

### StackScope

```cpp
StackScope scope{stack};   // captures marker on construction
                           // calls freeToMarker on destruction
```

---

## Benchmark Results

Benchmarks compare `Stack` against heap (`new` / `delete`) across all operations.
All times are total elapsed time for the listed iteration count.

> Compiled with `-std=c++23`. Results may vary depending on hardware and compiler optimizations.

### Allocation

```
----------------------------------------------------------------------
Allocation Benchmarks                   Time           Iteration
----------------------------------------------------------------------
Stack Allocate                          1.05 ms         1000000
Heap Allocate                           289.34 ms       1000000

Stack Allocate Aligned                  559.31 us       1000000
Heap Allocate Aligned                   265.27 ms       1000000

Stack Sequential                        1.58 ms         1000000
Heap Sequential                         629.16 ms       1000000

Stack Exhaustion Reset                  8.46 ms         1000000
Heap Exhaustion Reset                   4.29 s          1000000
----------------------------------------------------------------------
```

### Lifecycle

```
----------------------------------------------------------------------
Lifecycle Benchmarks                    Time           Iteration
----------------------------------------------------------------------
Stack Create Destroy Trivial            1.11 ms         1000000
Heap Create Destroy Trivial             295.56 ms       1000000

Stack Create Destroy Nontrivial         5.82 ms         1000000
Heap Create Destroy Nontrivial          277.89 ms       1000000

Stack Create With Args                  5.82 ms         1000000
Heap Create With Args                   262.39 ms       1000000
----------------------------------------------------------------------
```

### Marker

```
----------------------------------------------------------------------
Marker Benchmarks                       Time           Iteration
----------------------------------------------------------------------
Stack Marker Round Trip                 526.23 us       1000000
Heap Marker Round Trip                  217.30 ms       1000000

Stack Nested Markers                    2.06 ms         1000000
Heap Nested Markers                     728.09 ms       1000000
----------------------------------------------------------------------
```

### Scope

```
----------------------------------------------------------------------
Scope Benchmarks                        Time           Iteration
----------------------------------------------------------------------
Stack Scope                             1.50 ms         1000000
Stack Manual Marker                     1.05 ms         1000000
Heap New Delete                         298.10 ms       1000000

Stack Nested Scope                      1.12 ms         1000000
Stack Nested Manual Marker              1.05 ms         1000000
Heap Nested New Delete                  606.18 ms       1000000
----------------------------------------------------------------------
```

### Reset

```
----------------------------------------------------------------------
Reset Benchmarks                        Time           Iteration
----------------------------------------------------------------------
Stack Reset                             526.46 us       1000000
Heap Delete Cycle                       206.34 ms       1000000

Stack Exhaustion Reset                  8.45 ms         1000000
Heap Exhaustion Delete                  4.76 s          1000000

Stack Repeated Allocate Reset           1.58 ms         1000000
Heap Repeated New Delete                869.72 ms       1000000
----------------------------------------------------------------------
```

### Summary

Stack dominates heap across every allocation pattern due to its O(1) bump pointer strategy
and zero heap traffic after construction.

Single allocation (`Stack Allocate`: 1.05 ms vs `Heap Allocate`: 289.34 ms) shows a 275x
advantage — a bump pointer increment and mask vs heap search with fragmentation overhead.

Aligned allocation (`Stack Allocate Aligned`: 559 us vs `Heap Allocate Aligned`: 265.27 ms)
shows a 474x advantage — alignment is a single bitmask operation stored as a bit shift,
with no additional cost over unaligned allocation.

Marker round-trip (`Stack Marker Round Trip`: 526 us vs `Heap Marker Round Trip`: 217.30 ms)
shows a 413x advantage — restoring the offset is a single assignment vs heap free and
re-allocation with system call overhead.

Exhaustion and reset (`Stack Exhaustion Reset`: 8.46 ms vs `Heap Exhaustion Reset`: 4.29 s)
shows a 507x advantage — `reset()` is a single offset assignment vs N individual heap
deletes regardless of block count.

Scope overhead (`Stack Scope`: 1.50 ms vs `Stack Manual Marker`: 1.05 ms) is minimal —
the RAII wrapper adds roughly 0.45 ms per million operations compared to manual
`getMarker`/`freeToMarker`, both vastly outperforming heap at 298.10 ms.

Non-trivial lifecycle (`Stack Create Destroy Nontrivial`: 5.82 ms vs
`Heap Create Destroy Nontrivial`: 277.89 ms) shows a 47x advantage — the stack
eliminates heap search entirely, paying only construction and destruction cost.

| Category               | Winner | Notes                                               |
| ---------------------- | ------ | --------------------------------------------------- |
| Single allocate        | Stack  | 275x faster — bump pointer vs heap search           |
| Aligned allocate       | Stack  | 474x faster — bitmask alignment vs heap overhead    |
| Sequential allocations | Stack  | 398x faster — contiguous bump vs N heap calls       |
| Exhaustion reset       | Stack  | 507x faster — single assignment vs N heap deletes   |
| Trivial lifecycle      | Stack  | 266x faster — no heap traffic after construction    |
| Non-trivial lifecycle  | Stack  | 47x faster — stack eliminates heap search entirely  |
| Marker round-trip      | Stack  | 413x faster — offset restore vs heap free/alloc     |
| Nested markers         | Stack  | 353x faster — LIFO offset chain vs N heap ops       |
| Scope vs manual marker | Stack  | Minimal RAII overhead — 1.50 ms vs 1.05 ms          |
| Reset                  | Stack  | 392x faster — O(1) offset reset vs O(n) heap delete |

**Use Stack when:** allocation is linear, lifetimes are LIFO, or bulk reset patterns are needed.
**Use heap when:** object lifetimes are independent, sizes vary widely, or arbitrary deallocation order is required.

---

## Project Structure

```
StackAllocator/
├── include/
│   └── StackPro/
│       ├── Contract.h
│       ├── Stack.h
│       ├── Stack.tpp
│       └── StackScope.h
├── tests/
├── benchmarks/
├── examples/
├── cmake/
├── CMakeLists.txt
├── README.md
└── LICENSE
```

---

## Build Instructions

### Requirements

- GCC 16+ or Clang with C++23 support
- CMake 3.20+

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run Tests

```bash
./tests
```

### Run Benchmarks

```bash
./benchmarks
```

### Run Examples

```bash
./example_basic
./example_lifecycle
./example_scope
./example_stats
```

---

## Notes

- `reset()` does not call destructors. Caller is responsible for destroying live objects before reset.
- `freeToMarker()` reclaims all memory allocated after the marker — objects in that range are not destroyed.
- `destroy<T>()` calls the destructor but does not release memory — use `freeToMarker()` or `reset()` to reclaim.
- `getStats()` is only callable on `Stack<true>` — calling it on `Stack<false>` is a compile error.
- `StackScope` captures the marker at construction — allocations made before the scope are preserved.
- Alignment padding between allocations may cause `used()` to exceed the sum of requested sizes.

---

## License

[MIT](LICENSE) — free to use, modify, and distribute for educational and personal purposes.

