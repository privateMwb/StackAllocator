/**
 * @file            Stack.h
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

// clang-format off
#include <concepts>    // std::constructible_from
#include <cstddef>     // std::size_t, std::byte, std::max_align_t
#include <cstdint>     // std::uint8_t
#include <new>         // placement new, std::align_val_t
#include <type_traits> // std::conditional_t, std::is_array_v
#include <utility>     // std::forward, std::exchange
// clang-format on

#include <StackPro/Contract.h>

namespace StackPro {

/// @brief Forward declaration of the RAII stack scope helper.
template <bool EnableStats> class StackScope;

/**
 * @brief A linear (stack) allocator with O(1) allocate/reset/rollback.
 * @tparam EnableStats If `true`, tracks runtime allocation statistics
 * (total bytes allocated, current/peak usage, allocation count) at
 * zero size cost when `false`, via `[[no_unique_address]]`.
 * @details Allocations are carved sequentially from a single fixed-size
 * buffer obtained at construction. Memory can only be reclaimed in
 * bulk — either by resetting the whole allocator or rolling back to a
 * previously saved `Marker` — never by freeing an individual
 * allocation. There is no per-allocation bookkeeping (no header, no
 * free list), so `allocate()` is just a bump of an offset plus a
 * bounds check.
 */
template <bool EnableStats = false> class Stack {
  public:
    /**
     * @brief Opaque checkpoint of the allocator's offset, used to roll
     * back via `freeToMarker()`.
     * @details Carries no capacity or alignment information of its own —
     * only ever valid against the `Stack` instance that produced it.
     */
    struct Marker {
        /// @brief Returns the raw offset this marker captured.
        [[nodiscard]] constexpr std::size_t get() const noexcept {
            return value_;
        }

      private:
        friend class Stack;
        explicit constexpr Marker(std::size_t v) noexcept : value_{v} {}
        std::size_t
            value_; ///< Offset into the allocator's buffer at the time this marker was taken.
    };

    /// @brief Runtime allocation statistics. Present only when `EnableStats` is `true`.
    struct Stats {
        std::size_t totalAllocated_; ///< Total bytes allocated over the allocator's lifetime.
        std::size_t currentUsed_;    ///< Current bytes in use.
        std::size_t peakUsed_;       ///< Maximum bytes ever in use.
        std::size_t allocations_;    ///< Number of successful allocations.
    };

  private:
    /// @brief Zero-size placeholder used in place of `Stats` when statistics are disabled.
    struct Empty {};

    // Core allocator state.
    std::byte* memory_;       ///< Base of the owned buffer, or `nullptr` if moved-from.
    std::size_t cap_;         ///< Total size of the buffer, in bytes.
    std::size_t offset_;      ///< Current bump offset; also the number of bytes in use.
    std::uint8_t alignShift_; ///< `log2` of the alignment the buffer was allocated with.

    /// @brief Optional statistics storage with zero runtime overhead when disabled.
    [[no_unique_address]]
    std::conditional_t<EnableStats, Stats, Empty> stats_;

  public:
    /**
     * @brief Allocates a `size`-byte buffer aligned to `alignment` and
     * constructs an empty allocator over it.
     * @param size Size of the buffer to allocate, in bytes. Must be
     * greater than 0.
     * @param alignment Alignment of the underlying buffer. Must be a
     * power of two. Every per-call `allocate()`/`create()` alignment
     * must not exceed this value.
     * @throws std::bad_alloc if the underlying allocation fails.
     */
    explicit Stack(std::size_t size, std::size_t alignment = alignof(std::max_align_t));

    /// @brief Releases the underlying buffer.
    ~Stack();

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    /// @brief Move-constructs a stack, taking ownership of `other`'s
    /// buffer. `other` is left empty and reusable.
    Stack(Stack&& other) noexcept;

    /// @brief Move-assigns from `other`, releasing this allocator's
    /// buffer first. `other` is left empty and reusable.
    Stack& operator=(Stack&& other) noexcept;

    /**
     * @brief Allocates a block of memory from the allocator.
     * @param size Number of bytes to allocate. Must be greater than 0.
     * @param request_alignment Alignment of the returned block. Must be
     * a power of two and must not exceed the alignment given at
     * construction (enforced via `AP_PRE` in debug builds).
     * @return Pointer to the allocated block, or `nullptr` if the
     * allocator does not have enough remaining capacity.
     * @details O(1): rounds the current offset up to
     * `request_alignment`, bounds-checks against capacity, then bumps
     * the offset. Never fails for any reason other than insufficient
     * capacity.
     */
    [[nodiscard]] std::byte*
    allocate(std::size_t size, std::size_t request_alignment = alignof(std::max_align_t)) noexcept;

    /// @brief Returns the current allocation checkpoint.
    [[nodiscard]] Marker getMarker() const noexcept;

    /**
     * @brief Releases all allocations made after the specified checkpoint.
     * @param marker Checkpoint previously returned by `getMarker()` on
     * this same allocator. Must not be ahead of the current offset.
     */
    void freeToMarker(Marker marker) noexcept;

    /**
     * @brief Allocates storage for a `T` and constructs it in place.
     * @tparam T Object type to construct. Must not be an array type.
     * @tparam Args Deduced constructor argument types.
     * @param args Forwarded into `T`'s constructor.
     * @return Pointer to the newly constructed `T`, or `nullptr` if the
     * allocator does not have enough remaining capacity.
     * @details Storage is aligned to `alignof(T)`. If `T`'s constructor
     * throws, the bytes reserved for it are not reclaimed (equivalent
     * to a leaked allocation until the next `reset()`/`freeToMarker()`).
     */
    template <typename T, typename... Args>
        requires(!std::is_array_v<T>) && std::constructible_from<T, Args...>
    [[nodiscard]] T* create(Args&&... args);

    /**
     * @brief Destroys an object previously constructed with `create()`,
     * without reclaiming its storage.
     * @tparam T Object type to destroy. Must not be an array type.
     * @param ptr Pointer to destroy. If `nullptr`, this is a no-op.
     * @details Storage is only reclaimed via `reset()` or
     * `freeToMarker()` — this only runs `~T()`.
     */
    template <typename T>
        requires(!std::is_array_v<T>)
    void destroy(T* ptr) noexcept;

    /// @brief Resets the allocator to its initial (empty) state. Does
    /// not release the underlying buffer. Also clears statistics, if enabled.
    void reset() noexcept;

    /**
     * @brief Returns whether the pointer belongs to this allocator.
     * @param ptr Pointer to test.
     * @return `true` if `ptr` falls within `[memory_, memory_ + capacity())`.
     * @details Only checks range membership — not whether `ptr` is
     * currently live (i.e. before the current offset) or correctly aligned.
     */
    [[nodiscard]] AP_PURE bool owns(const void* ptr) const noexcept;

    /// @brief Returns runtime allocation statistics. Only available when `EnableStats` is `true`.
    [[nodiscard]] const Stats& getStats() const noexcept
        requires EnableStats;

    /// @brief Returns the number of bytes currently in use.
    [[nodiscard]] AP_PURE std::size_t used() const noexcept;
    /// @brief Returns the number of bytes remaining before the allocator is exhausted.
    [[nodiscard]] AP_PURE std::size_t remaining() const noexcept;
    /// @brief Returns the total capacity of the underlying buffer, in bytes.
    [[nodiscard]] AP_PURE std::size_t capacity() const noexcept;

  private:
    /**
     * @brief Allocates the underlying buffer for the allocator.
     * @param size Size of the buffer, in bytes. Must be greater than 0.
     * @param alignment Alignment to allocate the buffer with. Must be a
     * power of two.
     * @return Newly allocated buffer, aligned to `alignment`.
     * @throws std::bad_alloc if the allocation fails.
     */
    [[nodiscard]] static std::byte* allocateMemory(std::size_t size, std::size_t alignment);

    /**
     * @brief Rounds `ptr` up to the next multiple of `1 << shift`.
     * @param ptr Offset (or address) to align.
     * @param shift `log2` of the alignment to round up to.
     * @return `ptr` rounded up to the requested alignment.
     */
    [[nodiscard]] static constexpr std::size_t alignForward(std::size_t ptr,
                                                            std::uint8_t shift) noexcept;

    /**
     * @brief Converts a power-of-two alignment into its corresponding bit shift.
     * @param alignment Alignment value. Must be a power of two.
     * @return `log2(alignment)`.
     */
    [[nodiscard]] static constexpr std::uint8_t toShift(std::size_t alignment) noexcept;

    /// @brief Returns whether `value` is a power of two. `0` is not considered a power of two.
    [[nodiscard]] static constexpr bool isPowerOfTwo(std::size_t value) noexcept;

    /// @brief Records a successful allocation of `size` bytes, updating
    /// totals, current usage, peak usage, and count. No-op when
    /// `EnableStats` is `false`.
    constexpr void statAlloc(std::size_t size, std::size_t usedNow) noexcept;

    /// @brief Updates `currentUsed_` after a rollback. No-op when `EnableStats` is `false`.
    constexpr void statDealloc() noexcept;
};

} // namespace StackPro

/// @brief Umbrella alias so this library's types are reachable as
/// `rain::Stack`, alongside every other project library, while its true
/// namespace (and all internal diagnostics) remains `StackPro`. Reopens
/// `rain` rather than aliasing it, since multiple libraries each contribute
/// their own names into the same `rain` namespace -- an alias
/// (`namespace rain = StackPro;`) can only ever bind to one target and
/// collides the moment a second library declares its own `rain` alias to
/// something else. Declared here only, if StackScope.h includes this
/// header directly -- confirm against StackScope.h's own includes before
/// removing its declaration.
namespace rain {
using namespace StackPro;
}

#include "Stack.tpp"
