/**
 *	\file
 */

#pragma once

#include "../checked.hpp"
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <new>

namespace mcpp {
namespace test {

/**
 *	The state for a \ref allocator object.
 */
class allocator_state {
public:
	/**
	 *	The number of times \ref allocator::allocate
	 *	has been invoked for \ref allocator objects
	 *	associated with this object.
	 */
	std::size_t allocations;
	/**
	 *	The number of times \ref allocator::allocate
	 *	has been invoked for \ref allocator objects
	 *	associated with this object and has thrown
	 *	an exception.
	 */
	std::size_t failed_allocations;
	/**
	 *	The number of times \ref allocator::deallocate
	 *	has been invoked for \ref allocator objects
	 *	associated with this object.
	 */
	std::size_t deallocations;
	/**
	 *	The number of times bytes which have been allocated
	 *	through calls to \ref allocator::allocate for
	 *	\ref allocator objects associated with this
	 *	object.
	 */
	std::size_t allocated;
	/**
	 *	The number of bytes which have been deallocated
	 *	through calls to \ref allocator::deallocate for
	 *	\ref allocator objects associated with this
	 *	object.
	 */
	std::size_t deallocated;
	/**
	 *	The number of times \ref allocator objects
	 *	associated with this object have been rebound.
	 */
	std::size_t rebound;
	/**
	 *	If this member manages an exception that exception
	 *	will be thrown when \ref allocator::allocate is
	 *	called for \ref allocator objects associated
	 *	with this object.
	 */
	std::exception_ptr allocate_exception;
	allocator_state (const allocator_state &) = default;
	allocator_state (allocator_state &&) = default;
	allocator_state & operator = (const allocator_state &) = default;
	allocator_state & operator = (allocator_state &&) = default;
	allocator_state () noexcept;
};

/**
 *	An `Allocator` which defers to `std::malloc` and
 *	`std::free` but instruments calls to \ref allocate
 *	and \ref deallocate recording information about them
 *	to a \ref state.
 *
 *	\tparam T
 *		The type to allocate.
 */
template <typename T>
class allocator {
template <typename> friend class allocator;
public:
	using value_type = T;
	/**
	 *	Alias for \ref allocator_state.
	 *
	 *	Declared in this way to avoid having \ref allocator_state
	 *	depend on template parameters.
	 */
	using state = allocator_state;
private:
	state * state_;
	T * allocate_impl (std::size_t n) {
		if (state_->allocate_exception) std::rethrow_exception(state_->allocate_exception);
		auto result = checked::multiply(n, sizeof(T));
		if (!result) throw std::bad_alloc{};
		auto retr = static_cast<T *>(std::malloc(*result));
		if (!retr) throw std::bad_alloc{};
		state_->allocated += *result;
		return retr;
	}
public:
	allocator () = delete;
	allocator (const allocator &) = default;
	allocator (allocator &&) = default;
	allocator & operator = (const allocator &) = default;
	allocator & operator = (allocator &&) = default;
	/**
	 *	Creates an allocator which records information
	 *	about the memory it allocates to a certain
	 *	\ref state object.
	 *
	 *	\param [in] s
	 *		The \ref state object. Must remain valid for
	 *		the lifetime of this object and all objects
	 *		created therefrom or the behavior is undefined.
	 */
	explicit allocator (state & s) noexcept : state_(&s) {	}
	/**
	 *	Rebinds an allocator.
	 *
	 *	The rebound allocator will use the same \ref state
	 *	object.
	 *
	 *	\param [in] other
	 *		The allocator to rebind.
	 */
	template <typename U>
	explicit allocator (const allocator<U> & other) noexcept : state_(other.state_) {	}
	T * allocate (std::size_t n) {
		++state_->allocations;
		try {
			return allocate_impl(n);
		} catch (...) {
			++state_->failed_allocations;
			throw;
		}
	}
	void deallocate (T * ptr, std::size_t n) noexcept {
		++state_->deallocations;
		state_->deallocated += n * sizeof(T);
		std::free(ptr);
	}
	bool operator == (const allocator & rhs) const noexcept {
		return rhs.state_ == state_;
	}
	bool operator != (const allocator & rhs) const noexcept {
		return !(*this == rhs);
	}
};

}
}
