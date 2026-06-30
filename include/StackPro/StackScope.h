#pragma once

#include <StackPro/Stack.h>

namespace AllocatorPro {

// RAII helper that restores a stack allocator
// to its saved marker when leaving scope.
template<bool EnableStats =false>
class [[nodiscard]] StackScope {
public:

	// Constructor and destructor.
	explicit StackScope(Stack<EnableStats>& stack) noexcept
		: stack_ {
		stack
	}
	, marker_{stack.getMarker()}
	{}

	~StackScope() noexcept {
		stack_.freeToMarker(marker_);
	}

	StackScope(const StackScope&)            = delete;
	StackScope& operator=(const StackScope&) = delete;

	StackScope(StackScope&&)                 = delete;
	StackScope& operator=(StackScope&&)      = delete;

private:

	Stack<EnableStats>&                stack_;
	typename Stack<EnableStats>::Marker marker_;
};

} // namespace AllocatorPro
