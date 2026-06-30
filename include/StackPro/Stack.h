#pragma once

#include <StackPro/Contract.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

namespace AllocatorPro {

// Forward declaration of the RAII stack scope helper.
template<bool EnableStats>
class StackScope;

// A linear (stack) allocator.
// Allocations are performed sequentially and can only be reclaimed by
// resetting the allocator or rolling back to a previously saved marker.
template<bool EnableStats = false>
class Stack {
public:

	// Opaque checkpoint used to restore the allocator to a previous state.
	struct Marker {
		[[nodiscard]] constexpr std::size_t get() const noexcept {
			return value_;
		}
	private:
		friend class Stack;
		explicit constexpr Marker(std::size_t v) noexcept : value_ {
			v
		} {}
		std::size_t value_;
	};

	// Runtime allocation statistics.
	// Present only when EnableStats is true.
	struct Stats {
		std::size_t totalAllocated_; // Total bytes allocated over the allocator's lifetime.
		std::size_t currentUsed_;    // Current bytes in use.
		std::size_t peakUsed_;       // Maximum bytes ever in use.
		std::size_t allocations_;    // Number of successful allocations.
	};

private:

	// Zero-size placeholder used when statistics are disabled.
	struct Empty {};

	// Core allocator state.
	std::byte*    memory_;
	std::size_t   cap_;
	std::size_t   offset_;
	std::uint8_t  alignShift_;

	// Optional statistics storage with zero runtime overhead when disabled.
	[[no_unique_address]]
	std::conditional_t<EnableStats, Stats, Empty> stats_;

public:

	// Constructors and destructor.
	explicit Stack(std::size_t size,
	               std::size_t alignment = alignof(std::max_align_t));
	~Stack();

	Stack(const Stack&)             = delete;
	Stack& operator=(const Stack&)  = delete;

	Stack(Stack&& other)             noexcept;
	Stack& operator=(Stack&& other)  noexcept;

	// Allocates a block of memory from the allocator.
	[[nodiscard]] std::byte* allocate(
	    std::size_t size,
	    std::size_t request_alignment = alignof(std::max_align_t)) noexcept;

	// Returns the current allocation checkpoint.
	[[nodiscard]] Marker getMarker() const noexcept;

	// Releases all allocations made after the specified checkpoint.
	void freeToMarker(Marker marker) noexcept;

	// Object construction and destruction utilities.
	template<typename T, typename... Args>
	requires (!std::is_array_v<T>) && std::constructible_from<T, Args...>
	[[nodiscard]] T* create(Args&&... args);

	template<typename T>
	requires (!std::is_array_v<T>)
	void destroy(T* ptr) noexcept;

	// Resets the allocator to its initial state.
	void reset() noexcept;

	// Returns whether the pointer belongs to this allocator.
	[[nodiscard]] AP_PURE bool owns(const void* ptr) const noexcept;

	// Returns runtime allocation statistics.
	[[nodiscard]] const Stats& getStats() const noexcept
	requires EnableStats;

	// Returns allocator usage information.
	[[nodiscard]] AP_PURE std::size_t used()       const noexcept;
	[[nodiscard]] AP_PURE std::size_t remaining()  const noexcept;
	[[nodiscard]] AP_PURE std::size_t capacity()   const noexcept;

private:

	// Memory Allocation helper
	[[nodiscard]] static std::byte* allocateMemory(std::size_t size, std::size_t alignment);

	// Alignment helper utilities.
	[[nodiscard]] static constexpr std::size_t   alignForward(std::size_t ptr, std::uint8_t shift)  noexcept;
	[[nodiscard]] static constexpr std::uint8_t  toShift(std::size_t alignment)                     noexcept;
	[[nodiscard]] static constexpr bool          isPowerOfTwo(std::size_t value)                    noexcept;

	// Internal statistics helpers.
	constexpr void statAlloc(std::size_t size, std::size_t usedNow) noexcept;
	constexpr void statDealloc() noexcept;
};

} // namespace AllocatorPro

#include "Stack.tpp"
