#pragma once

#include <memory_resource>

// The standard implementation benchmarked against StackPro — the
// standard library's own linear/bump allocator.
using stdStack = std::pmr::monotonic_buffer_resource;