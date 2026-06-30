// ===========================================================
// Stack.tpp
// Template implementation for AllocatorPro::Stack.
// ===========================================================
//
//  Sections:
//   1. Constructors & Destructor
//   2. Move Semantics
//   3. Memory Management
//   4. Object Lifecycle
//   5. State Management
//   6. Introspection
//   7. Memory Allocation Helper
//   8. Alignment Utilities
//   9. Statistics Helpers
//
// ============================================================

#include <bit>
#include <new>

namespace AllocatorPro {

// ============================================================
//  Section 1 — Constructors & Destructor
// ============================================================

template<bool EnableStats>
Stack<EnableStats>::Stack(std::size_t size, std::size_t alignment)
	: memory_      {allocateMemory(size, alignment)}
	, cap_         {size}
	, offset_      {0}
	, alignShift_  {toShift(alignment)}
	, stats_       {} {}


template<bool EnableStats>
Stack<EnableStats>::~Stack() {
	::operator delete(memory_, std::align_val_t{std::size_t{1} << alignShift_});
}


// ============================================================
//  Section 2 — Move Semantics
// ============================================================

template<bool EnableStats>
Stack<EnableStats>::Stack(Stack&& other) noexcept
	: memory_  {
	std::exchange(other.memory_, nullptr)
}
, cap_         {std::exchange(other.cap_, 0)}
, offset_      {std::exchange(other.offset_, 0)}
, alignShift_  {std::exchange(other.alignShift_, 0)}
, stats_{std::exchange(other.stats_, decltype(other.stats_) {})} {}

template<bool EnableStats>
Stack<EnableStats>& Stack<EnableStats>::operator=(Stack&& other) noexcept {
	if (this != &other) {
		::operator delete(memory_, std::align_val_t{std::size_t{1} << alignShift_});

		memory_      = std::exchange(other.memory_, nullptr);
		cap_         = std::exchange(other.cap_, 0);
		offset_      = std::exchange(other.offset_, 0);
		alignShift_  = std::exchange(other.alignShift_, 0);
		stats_       = std::exchange(other.stats_, decltype(other.stats_) {});
	}
	return *this;
}


// ============================================================
//  Section 3 — Memory Management
// ============================================================

template<bool EnableStats>
std::byte* Stack<EnableStats>::allocate(std::size_t size, std::size_t requestAlignment) noexcept {

	// Validate arguments.
	AP_PRE(size > 0);
	AP_PRE(isPowerOfTwo(requestAlignment));

	// Compute the aligned allocation offset.
	const std::size_t alignedOffset = alignForward(offset_, toShift(requestAlignment));

	// Check available capacity.
	if (alignedOffset + size > cap_) return nullptr;

	// Reserve the requested memory block.
	std::byte* ptr = memory_ + alignedOffset;
	offset_ = alignedOffset + size;

	// Update allocation statistics.
	statAlloc(size, offset_);

	// Return the allocated block.
	return ptr;
}

template<bool EnableStats>
auto Stack<EnableStats>::getMarker() const noexcept -> Marker {
	return Marker{offset_};
}

template<bool EnableStats>
void Stack<EnableStats>::freeToMarker(Marker marker) noexcept {

	// Validate marker.
	AP_PRE(marker.get() <= offset_);

	// Restore the allocation state.
	offset_ = marker.get();

	// Update allocation statistics.
	statDealloc();
}


// ============================================================
//  Section 4 — Object Lifecycle
// ============================================================

template<bool EnableStats>
template<typename T, typename... Args>
requires (!std::is_array_v<T>) && std::constructible_from<T, Args...>
T* Stack<EnableStats>::create(Args&&... args) {

	// Reserve memory for the object.
	std::byte* memory = allocate(sizeof(T), alignof(T));

	// Return nullptr if the allocation failed.
	if (!memory) return nullptr;

	// Construct the object in the allocated storage.
	return ::new (memory) T(std::forward<Args>(args)...);
}

template<bool EnableStats>
template<typename T>
requires (!std::is_array_v<T>)
void Stack<EnableStats>::destroy(T* ptr) noexcept {

	// Ignore null pointers
	if (ptr == nullptr) return;

	// Destroy the object without releasing its storage.
	ptr->~T();
}


// ============================================================
//  Section 5 — State Management
// ============================================================

template<bool EnableStats>
void Stack<EnableStats>::reset() noexcept {
	offset_ = 0;

	if constexpr (EnableStats)
		stats_ = {};
}


// ============================================================
//  Section 6 — Introspection
// ============================================================

template<bool EnableStats>
bool Stack<EnableStats>::owns(const void* ptr) const noexcept {
	const auto* p = static_cast<const std::byte*>(ptr);

	return memory_ <= p && p < memory_ + cap_;
}

template<bool EnableStats>
auto Stack<EnableStats>::getStats() const noexcept -> const Stats&
requires EnableStats {
	return stats_;
}

template<bool EnableStats>
std::size_t Stack<EnableStats>::used() const noexcept {
	return offset_;
}

template<bool EnableStats>
std::size_t Stack<EnableStats>::remaining() const noexcept {
	return cap_ - offset_;
}

template<bool EnableStats>
std::size_t Stack<EnableStats>::capacity() const noexcept {
	return cap_;
}


// ============================================================
//  Section 7 — Memory Allocation Helper
// ============================================================

template<bool EnableStats>
std::byte* Stack<EnableStats>::allocateMemory(std::size_t size, std::size_t alignment) {
	AP_PRE(size > 0);
	AP_PRE(isPowerOfTwo(alignment));

	return static_cast<std::byte*>(::operator new(size, std::align_val_t{alignment}));
}


// ============================================================
//  Section 8 — Alignment Utilities
// ============================================================

template<bool EnableStats>
constexpr std::size_t Stack<EnableStats>::alignForward(std::size_t ptr, std::uint8_t shift) noexcept {

	// Round the address up to the next aligned boundary.
	const std::size_t mask = (std::size_t{1} << shift) - 1u;
	return (ptr + mask) & ~mask;
}

template<bool EnableStats>
constexpr std::uint8_t Stack<EnableStats>::toShift(std::size_t alignment) noexcept {

	// Convert a power-of-two alignment into its corresponding bit shift.
	AP_PRE(isPowerOfTwo(alignment));
	return static_cast<std::uint8_t>(std::countr_zero(alignment));
}

template<bool EnableStats>
constexpr bool Stack<EnableStats>::isPowerOfTwo(std::size_t val) noexcept {
	return val != 0 && (val & (val - 1)) == 0;
}


// ============================================================
//  Section 9 — Statistics Helper
// ============================================================

template<bool EnableStats>
constexpr void Stack<EnableStats>::statAlloc(std::size_t size, std::size_t usedNow) noexcept {
	if constexpr (EnableStats) {
		stats_.totalAllocated_  += size;
		stats_.currentUsed_     = usedNow;
		++stats_.allocations_;

		if (usedNow > stats_.peakUsed_)
			stats_.peakUsed_ = usedNow;
	}
}

template<bool EnableStats>
constexpr void Stack<EnableStats>::statDealloc() noexcept {
	if constexpr (EnableStats)
		stats_.currentUsed_ = offset_;
}

} // namespace AllocatorPro