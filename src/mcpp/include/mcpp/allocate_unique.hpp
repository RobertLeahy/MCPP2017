/**
 *	\file
 */

#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace mcpp {

namespace detail {

template <typename Allocator>
class allocate_unique_deleter {
static_assert(
	std::is_nothrow_move_constructible<Allocator>::value,
	"Allocator must be nothrow move constructible"
);
static_assert(
	std::is_nothrow_move_assignable<Allocator>::value,
	"Allocator must be nothrow move assignable"
);
private:
	Allocator a_;
	using traits = std::allocator_traits<Allocator>;
public:
	using pointer = typename traits::pointer;
	allocate_unique_deleter () = default;
	allocate_unique_deleter (const allocate_unique_deleter &) = delete;
	allocate_unique_deleter (allocate_unique_deleter &&) = default;
	allocate_unique_deleter & operator = (const allocate_unique_deleter &) = delete;
	allocate_unique_deleter & operator = (allocate_unique_deleter &&) = default;
	explicit allocate_unique_deleter (const Allocator & a) noexcept(
		std::is_nothrow_copy_constructible<Allocator>::value
	)	:	a_(a)
	{	}
	template <typename T>
	void operator () (T * ptr) noexcept {
		if (!ptr) return;
		traits::destroy(a_, ptr);
		traits::deallocate(a_, ptr, 1);
	}
	template <typename OtherAllocator>
	operator allocate_unique_deleter<OtherAllocator> () const noexcept {
		using value_type = typename std::allocator_traits<OtherAllocator>::value_type;
		using rebound_type = typename std::allocator_traits<Allocator>::template rebind_alloc<value_type>;
		rebound_type a(a_);
		return allocate_unique_deleter<OtherAllocator>(a);
	}
};

template <typename T, typename Allocator>
using allocate_unique_allocator_t = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;

}

/**
 *	The type returned by \ref allocate_unique.
 *
 *	\tparam T
 *		The type of the pointee.
 *	\tparam Allocator
 *		The `Allocator` which shall be used to
 *		allocate and deallocate memory.
 */
template <typename T, typename Allocator>
using allocate_unique_t = std::unique_ptr<
	T,
	detail::allocate_unique_deleter<
		detail::allocate_unique_allocator_t<T, Allocator>
	>
>;

/**
 *	Emulates the functionality of `std::make_unique`
 *	except uses an `Allocator` to allocate memory
 *	and construct the pointee.
 *
 *	\tparam T
 *		The type of the pointee.
 *	\tparam Allocator
 *		The `Allocator` which shall be used to
 *		allocate and deallocate memory.
 *	\tparam Args
 *		Arguments which shall be forwarded through
 *		to a constructor of \em T.
 *
 *	\param [in] a
 *		The allocator to use.
 *	\param [in] args
 *		The arguments to forward through to a constructor
 *		of \em T.
 *
 *	\return
 *		A `std::unique_ptr` with an appropriate `Deleter`
 *		to deallocate the allocated memory using \em a.
 */
template <typename T, typename Allocator, typename... Args>
allocate_unique_t<T, Allocator> allocate_unique (const Allocator & a, Args &&... args) {
	static_assert(!std::is_array<T>::value, "T must not be array");
	using allocator = detail::allocate_unique_allocator_t<T, Allocator>;
	allocator rebound(a);
	detail::allocate_unique_deleter<allocator> d(rebound);
	using traits = std::allocator_traits<allocator>;
	typename traits::pointer p = traits::allocate(rebound, 1);
	try {
		traits::construct(rebound, p, std::forward<Args>(args)...);
	} catch (...) {
		traits::deallocate(rebound, p, 1);
		throw;
	}
	return allocate_unique_t<T, Allocator>(p, std::move(d));
}

}
