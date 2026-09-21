/**
 * @file            StackScope.h
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

#include <StackPro/Stack.h>

namespace StackPro {

/**
 * @brief RAII helper that restores a stack allocator to its saved
 * marker when leaving scope.
 * @tparam EnableStats Must match the `Stack` specialization being scoped.
 * @details Captures `stack.getMarker()` on construction and calls
 * `stack.freeToMarker()` with it on destruction, reclaiming everything
 * allocated during the scope's lifetime. Non-copyable and non-movable —
 * a scope is tied to the exact point in the call stack where it was created.
 */
template <bool EnableStats = false> class [[nodiscard]] StackScope {
  public:
    /**
     * @brief Captures `stack`'s current marker.
     * @param stack Allocator to scope. Must outlive this `StackScope`.
     */
    explicit StackScope(Stack<EnableStats>& stack) noexcept
        : stack_{stack}, marker_{stack.getMarker()} {}

    /// @brief Rolls `stack_` back to the marker captured at construction.
    ~StackScope() noexcept {
        stack_.freeToMarker(marker_);
    }

    StackScope(const StackScope&) = delete;
    StackScope& operator=(const StackScope&) = delete;

    StackScope(StackScope&&) = delete;
    StackScope& operator=(StackScope&&) = delete;

  private:
    Stack<EnableStats>& stack_; ///< Allocator being scoped.
    typename Stack<EnableStats>::Marker
        marker_; ///< Marker captured at construction, restored at destruction.
};

} // namespace StackPro
