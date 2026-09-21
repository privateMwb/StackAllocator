/**
 * @file            Contract.h
 *
 * @date            2026-07-30
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 privateMwb
 *                  All rights reserved.
 *                  https://github.com/privateMwb/StackAllocator
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

#include <cassert> // assert

// ============================================================================
// Contract macros.
//
// By default, all contract macros map to assert(). Their implementation
// can be replaced globally without modifying library source code.
//
// In release builds (NDEBUG defined), all contract macros expand to
// no-ops. Callers are responsible for satisfying preconditions
// unconditionally.
// ============================================================================

/// @def AP_PRE
/// @brief Documents a precondition that callers must satisfy.
#define AP_PRE(condition) assert(condition)

/// @def AP_POST
/// @brief Documents a postcondition guaranteed by the function.
#define AP_POST(condition) assert(condition)

/// @def AP_INVARIANT
/// @brief Documents a condition that must always hold for an object's state.
#define AP_INVARIANT(condition) assert(condition)

/// @def AP_ASSERT
/// @brief An internal implementation assertion, not part of the public contract.
#define AP_ASSERT(condition) assert(condition)

// ============================================================================
// Compiler attributes.
// ============================================================================

#if defined(__GNUC__) || defined(__clang__)
/// @def AP_PURE
/// @brief Marks a function as having no observable side effects, with
/// its return value depending only on its arguments and/or object
/// state. Enables additional compiler optimizations when supported.
#define AP_PURE __attribute__((pure))
#else
/// @def AP_PURE
/// @brief No-op on compilers without an equivalent attribute.
#define AP_PURE
#endif
