/**
 * @file Contract.h
 * @brief Contract macros and compiler attributes shared across StackPro.
 *
 * Documents function preconditions, postconditions, and invariants via
 * macros that map to `assert()` by default, plus small compiler-attribute
 * wrappers used for portability. This header defines no namespace — its
 * macros are meant to be used unqualified throughout the library.
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
