# StackAllocator

<p align="center">
  <img src="https://img.shields.io/github/v/release/privateMwb/StackAllocator?style=for-the-badge&logo=github&color=yellow" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-orange?style=for-the-badge" alt="License - MIT">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue?style=for-the-badge&logo=c%2B%2B" alt="C++ - 20">
</p>

<p align="center">
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/build.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/build.yml/badge.svg" alt="Build and Test">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/benchmark.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/benchmark.yml/badge.svg" alt="Benchmarks">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/coverage.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/coverage.yml/badge.svg" alt="Coverage">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/sanitizers.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/sanitizers.yml/badge.svg" alt="Sanitizers">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/clang-tidy.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/clang-tidy.yml/badge.svg" alt="Clang Tidy">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/clang-format.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/clang-format.yml/badge.svg" alt="Clang Format">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/docs.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/release.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/release.yml/badge.svg" alt="Release">
  </a>
  <a href="https://github.com/privateMwb/StackAllocator/actions/workflows/packaging.yml">
    <img src="https://github.com/privateMwb/StackAllocator/actions/workflows/packaging.yml/badge.svg" alt="Packaging">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/GCC-support-B46F1B?style=flat&logo=gnu" alt="GCC - support">
  <img src="https://img.shields.io/badge/Clang-support-045891?style=flat&logo=llvm" alt="Clang - support">
  <img src="https://img.shields.io/badge/MSVC-support-5C2D91?style=flat" alt="MSVC - support">
  <img src="https://img.shields.io/badge/AppleClang-support-000000?style=flat&logo=apple" alt="AppleClang - support">
</p>

StackAllocator is a header-only, fixed-capacity bump-pointer stack allocator for modern C++ — O(1) `allocate()`/`create()`, a single buffer allocated once at construction instead of per-allocation heap traffic, and marker-based, LIFO rollback via `StackScope` instead of tracking and freeing objects one at a time.

## 📑 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Development](#development)
- [Benchmarks](#benchmarks)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

## <a id="features"></a>✨ Features

- **Single upfront buffer, bump-pointer allocation** — the whole stack is allocated once at construction; `allocate()`/`create<T>()` are just a few arithmetic operations advancing an offset, so steady-state allocation never touches the heap again.
- **Marker-based rollback via `StackScope`** — `getMarker()` captures a checkpoint and `freeToMarker()` rolls back to it, with no fixed nesting-depth limit; `StackScope` wraps that in RAII so scratch allocations are automatically reclaimed — even when an exception unwinds through the scope — without freeing anything individually.
- **In-place construction and destruction** — `create<T>()` forwards its arguments directly into `T`'s constructor inside the stack; `destroy()` runs `T`'s destructor without moving the offset, since the stack reclaims space in bulk (`freeToMarker()`/`reset()`), not per object.
- **Alignment-aware, contract-based API** — `allocate()` honors an explicit alignment, including over-aligned types, bounded by a per-instance ceiling set at construction, and preconditions across the API are documented and enforced via assert-based contracts, consistent everywhere rather than mixed error-handling styles.
- **Pointer provenance checks** — `owns()` answers whether a given pointer falls within a specific stack's buffer, useful for validating input at a subsystem boundary without trusting callers to tag their pointers correctly.
- **Optional, zero-cost statistics** — a compile-time `EnableStats` flag adds allocation/usage tracking (`getStats()`) with zero overhead when disabled.

## <a id="requirements"></a>📋 Requirements

- A C++20-conformant compiler (tested: GCC, Clang, MSVC, AppleClang)
- CMake 3.20+

## <a id="installation"></a>📦 Installation

**From source:**

```bash
git clone https://github.com/privateMwb/StackAllocator.git
cd StackAllocator
cmake -B build \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_REGRESSION=OFF \
  -DBUILD_EXAMPLES=OFF
cmake --install build
```

Then, in your own `CMakeLists.txt`:

```cmake
find_package(StackPro CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE StackPro::StackPro)
```

> vcpkg and Conan packages are built and verified (recipe in
> `packaging/recipes/stackpro/`, port in `packaging/vcpkg/ports/stackpro/`),
> but not yet published to the public registries. This section will be
> updated once they are.

## <a id="quick-start"></a>🚀 Quick Start

```cpp
#include <StackPro/Stack.h>

int main() {
    StackPro::Stack<> stack(1024);

    std::byte* raw = stack.allocate(64);         // raw bytes
    auto* widget = stack.create<Widget>(1, 2);   // constructed in place

    stack.destroy(widget); // runs ~Widget(); storage stays reserved
}
```

Scratch work scoped to a single call, rolled back automatically:

```cpp
#include <StackPro/Stack.h>
#include <StackPro/StackScope.h>

void process(StackPro::Stack<>& stack) {
    StackPro::StackScope scope(stack); // captures a marker

    stack.allocate(256); // scratch space for this call only
    // ... rolls back automatically when scope goes out of scope,
    // even if an exception unwinds through it
}
```

Tracking usage with statistics enabled:

```cpp
StackPro::Stack<true> stack(4096); // EnableStats = true

stack.allocate(128);

const auto& stats = stack.getStats();
std::cout << stats.allocations_ << " allocations, "
          << stats.peakUsed_ << " bytes at peak\n";
```

## <a id="project-structure"></a>🗂️ Project Structure

```
StackAllocator/
├── include/
│   └── StackPro/
│       ├── Stack.h
│       ├── Stack.tpp
│       ├── StackScope.h
│       └── Contract.h
│
├── tests/
│   ├── support/
│   ├── suite/
│   ├── test_main.cpp
│   └── CMakeLists.txt
│
├── benchmarks/
│   ├── support/
│   ├── suite/
│   ├── baselines/
│   ├── bench_main.cpp
│   └── CMakeLists.txt
│
├── examples/
│   ├── support/
│   ├── suite/
│   ├── example_main.cpp
│   └── CMakeLists.txt
│
├── regression/
│   ├── support/
│   ├── regression_main.cpp
│   └── CMakeLists.txt
│
├── packaging/
│   ├── README.md
│   ├── recipes/
│   │   └── stackpro/
│   ├── vcpkg/
│   │   └── ports/
│   │       └── stackpro/
│   └── vcpkg-smoke-test/
│
├── scripts/
│   └── update_package_files.py
│
├── .github/
│   ├── releases/
│   └── workflows/
│
├── cmake/
│   └── StackProConfig.cmake.in
│
├── docs/
│   ├── Doxyfile
│   └── README.md
│
├── .gitignore
├── CMakeLists.txt
├── README.md
└── LICENSE
```

## <a id="development"></a>🛠️ Development

The from-source install above builds the library only. To work on
StackPro itself — running tests, benchmarks, or the regression tool —
build with everything enabled (the default):

```bash
cmake -B build
cmake --build build
```

**Run the test suite:**

```bash
ctest --test-dir build
```

**Run benchmarks and check for regressions:**

```bash
./build/benchmarks
./build/regression                  # latest baseline vs. benchmarks/results/benchmark_results.json
./build/regression v1.2.0           # a specific baseline vs. current
./build/regression v1.2.0 v1.4.0    # two baselines against each other
```

`regression` picks the latest baseline by semantic version (`v1.10.0`
correctly outranks `v1.9.0`), not alphabetical filename order, and
auto-names its output (`regression_v1.2.0_vs_current.md`/`.json`, etc.).

See [packaging/README.md](packaging/README.md) for notes on verifying the vcpkg
port and Conan recipe locally.

## <a id="benchmarks"></a>📊 Benchmarks

Measured against `stdStack` (a naive, dynamically-growing baseline),
same build, at 10K / 100K / 1M iterations (`benchmarks/baselines/v1.0.0.json`
has the full dataset).

| Operation | StackPro (1M) | stdStack (1M) | Δ |
|---|---|---|---|
| `Allocate() @ 64 MiB Buffer` | 308.33 us | 17.38 s | +5635372.3% |
| `Allocate() 4096-byte Aligned` | 309.12 us | 11.10 s | +3589567.1% |
| `Allocate() At Capacity (Failure Path)` | 308.35 us | 1.78 s | +578002.8% |
| `Reset() Alone` | 308.34 us | 953.03 us | +209.1% |
| `Reset() + Refill` | 65.48 ms | 128.47 ms | +96.2% |
| `Allocate() Large` | 660.43 us | 1.28 ms | +93.2% |
| `Allocate() Over-aligned` | 753.58 us | 1.29 ms | +70.8% |
| `Create<T>() Trivial Ctor` | 1.31 ms | 1.79 ms | +36.9% |
| `Create<T>() Non-trivial Ctor` | 2.51 ms | 3.37 ms | +33.9% |
| `Destroy<T>() Non-trivial Dtor` | 639.55 us | 647.51 us | +1.2% |
| `Destroy<T>() Trivial Dtor` | 308.33 us | 308.31 us | -0.0% |
| `Construction` | 29.96 ms | 7.48 ms | -75.0% |

StackPro's fixed-buffer, bump-pointer design pays off most on the
exhaustion path (a bounds check and a `nullptr` return versus
`stdStack` actually reallocating and copying), large or over-aligned
buffers, and bulk churn (`Reset()` + refill), where `stdStack`'s
per-call bookkeeping and growth strategy show up directly. The
alignment and capacity-growth results are the most dramatic:
`stdStack`'s cost scales with buffer size and requested alignment,
while StackPro's stays flat regardless of either, since it never
reallocates.

The trade-off: the buffer is allocated once, eagerly and aligned, at
construction — so `Construction` is consistently slower than
`stdStack`'s lazy setup. `Destroy()` is roughly a wash either way,
since both implementations ultimately just run the same destructor;
StackPro adds nothing on top of that, but it doesn't reclaim anything
faster either.

## <a id="documentation"></a>📖 Documentation

Full API reference, generated with Doxygen from `docs/Doxyfile`:

**https://privateMwb.github.io/StackAllocator/**

## <a id="contributing"></a>🤝 Contributing

Issues and pull requests are welcome. Before submitting a PR:

- Run the test suite (`ctest --test-dir build`)
- If you're changing a hot path, run `./build/regression` and mention
  the results in your PR description

## <a id="changelog"></a>📝 Changelog

See the [Releases](https://github.com/privateMwb/StackAllocator/releases)
page for version history and release notes.

## <a id="license"></a>📄 License

MIT — see [LICENSE](LICENSE) for details.
