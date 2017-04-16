/**
 *	\file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mcpp {

namespace detail {

template <typename T>
class polymorphic_ptr_malloc_base {
protected:
	static void deallocate (void * ptr, std::size_t) noexcept {
		std::free(ptr);
	}
	static std::pair<void *, std::size_t> allocate (std::size_t n) {
		auto retr = std::malloc(n);
		if (!retr) throw std::bad_alloc{};
		return std::make_pair(retr, n);
	}
	static std::pair<void *, std::size_t> reallocate (void * ptr, std::size_t, std::size_t n) {
		auto retr = std::realloc(ptr, n);
		if (!retr) throw std::bad_alloc{};
		return std::make_pair(retr, n);
	}
};

template <typename T, typename Allocator>
class polymorphic_ptr_allocator_base {
private:
	using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<std::max_align_t>;
	using traits_type = std::allocator_traits<allocator_type>;
	static constexpr bool is_nothrow_constructible = std::is_nothrow_constructible<allocator_type, const Allocator &>::value;
	allocator_type a_;
	static std::size_t convert_size(std::size_t bytes) noexcept {
		constexpr std::size_t size = sizeof(std::max_align_t);
		std::size_t retr = bytes / size;
		if ((bytes % size) != 0) ++retr;
		return retr;
	}
protected:
	void deallocate (void * ptr, std::size_t n) {
		traits_type::deallocate(a_, reinterpret_cast<std::max_align_t *>(ptr), n / sizeof(std::max_align_t));
	}
	std::pair<void *, std::size_t> allocate (std::size_t n) {
		auto size = convert_size(n);
		return std::make_pair(traits_type::allocate(a_, size), size * sizeof(std::max_align_t));
	}
	std::pair<void *, std::size_t> reallocate (void * ptr, std::size_t prev, std::size_t n) {
		auto pair = allocate(n);
		deallocate(ptr, prev);
		return pair;
	}
public:
	polymorphic_ptr_allocator_base () = default;
	explicit polymorphic_ptr_allocator_base (const Allocator & a) noexcept(is_nothrow_constructible)
		:	a_(a)
	{	}
};

class polymorphic_ptr_malloc_tag {	};

template <typename T>
class polymorphic_ptr_storage_base {
public:
	void * raw;
	std::size_t size;
	T * object;
	polymorphic_ptr_storage_base () noexcept
		:	raw(nullptr),
			size(0),
			object(nullptr)
	{	}
	~polymorphic_ptr_storage_base () noexcept {
		assert(!raw);
		assert(size == 0);
		assert(!object);
	}
	polymorphic_ptr_storage_base (const polymorphic_ptr_storage_base &) = delete;
	polymorphic_ptr_storage_base & operator = (const polymorphic_ptr_storage_base &) = delete;
	polymorphic_ptr_storage_base (polymorphic_ptr_storage_base && other) noexcept
		:	raw(other.raw),
			size(other.size),
			object(other.object)
	{
		other.raw = nullptr;
		other.size = 0;
		other.object = nullptr;
	}
	polymorphic_ptr_storage_base & operator = (polymorphic_ptr_storage_base && rhs) noexcept {
		raw = rhs.raw;
		size = rhs.size;
		object = rhs.object;
		rhs.raw = nullptr;
		rhs.size = 0;
		rhs.object = nullptr;
		return *this;
	}
	template <typename U, typename... Args>
	U & emplace (Args &&... args) noexcept(std::is_nothrow_constructible<U, Args &&...>::value) {
		static_assert(std::is_base_of<T, U>::value, "U must derive from T");
		auto retr = new (raw) U (std::forward<Args>(args)...);
		object = retr;
		return *retr;
	}
	void destroy () noexcept(std::is_nothrow_destructible<T>::value) {
		if (object) object->~T();
		object = nullptr;
	}
};

template <typename T>
class polymorphic_ptr_no_virtual_dtor_storage : public polymorphic_ptr_storage_base<T> {
private:
	using base = polymorphic_ptr_storage_base<T>;
	static constexpr bool is_nothrow_destructible = std::is_nothrow_destructible<T>::value;
	using destroy_type = void (*) (T *);
	destroy_type dtor_;
	template <typename U>
	static void cleanup (T * ptr) noexcept(is_nothrow_destructible) {
		static_cast<U *>(ptr)->~U();
	}
public:
	polymorphic_ptr_no_virtual_dtor_storage () noexcept : dtor_(nullptr) {	}
	template <typename U, typename... Args>
	U & emplace (Args &&... args) noexcept(std::is_nothrow_constructible<U, Args &&...>::value) {
		auto & retr = base::template emplace<U>(std::forward<Args>(args)...);
		dtor_ = &cleanup<U>;
		return retr;
	}
	void destroy () noexcept(is_nothrow_destructible) {
		if (base::object) dtor_(base::object);
		base::object = nullptr;
		dtor_ = nullptr;
	}
};

template <typename T, typename = std::true_type>
class polymorphic_ptr_storage : public polymorphic_ptr_no_virtual_dtor_storage<T> {	};
template <typename T>
class polymorphic_ptr_storage<T, typename std::has_virtual_destructor<T>::type> : public polymorphic_ptr_storage_base<T> {	};

template <typename T, typename Allocator>
class polymorphic_ptr_base : public polymorphic_ptr_allocator_base<T, Allocator> {
private:
	using base = polymorphic_ptr_allocator_base<T, Allocator>;
public:
	using base::base;
};
template <typename T>
class polymorphic_ptr_base<T, polymorphic_ptr_malloc_tag> : public polymorphic_ptr_malloc_base<T> {
private:
	using base = polymorphic_ptr_malloc_base<T>;
public:
	using base::base;
};

}

/**
 *	Maintains a buffer in which objects all of which are
 *	polymorphic with some base class may reside. The buffer
 *	is maintained between the lifetimes of objects thereby
 *	allowing it to be reused between subsequent objects
 *	if it is large enough. If it is not large enough it is
 *	resized and this ongoing process eventually causes
 *	allocations to cease which is of great benefit when
 *	working with a large number of short-lived polymorphic
 *	objects.
 *
 *	Also unlike `delete` on raw pointers this class
 *	template is capable of safely managing the lifetimes
 *	of derived instances even when the base class lacks
 *	a virtual destructor. Note that this incurs a size
 *	overhead as the object must track the most derived
 *	type of the managed object. This size overhead is not
 *	paid when \em T has a virtual destructor.
 *
 *	\tparam T
 *		The type of the base class.
 *	\tparam Allocator
 *		The allocator to use to obtain memory. Defaults to
 *		a special tag type which will cause the resulting
 *		class to use `std::malloc` and `std::free`. This
 *		enables an optimization wherein the class may use
 *		`std::realloc` to grow memory as the C++ `Allocator`
 *		concept does not support reallocation only allocation
 *		and deallocation.
 */
template <typename T, typename Allocator = detail::polymorphic_ptr_malloc_tag>
class polymorphic_ptr : private detail::polymorphic_ptr_base<T, Allocator> {
private:
	using base = detail::polymorphic_ptr_base<T, Allocator>;
	detail::polymorphic_ptr_storage<T> storage_;
	static constexpr bool is_nothrow_destructible = std::is_nothrow_destructible<T>::value;
	void destroy () noexcept(is_nothrow_destructible) {
		storage_.destroy();
	}
	void deallocate () noexcept(is_nothrow_destructible) {
		destroy();
		base::deallocate(storage_.raw, storage_.size);
		storage_.raw = nullptr;
		storage_.size = 0;
	}
	template <typename U>
	void reallocate () {
		destroy();
		constexpr std::size_t size = sizeof(U);
		if (storage_.size >= size) return;
		std::tie(storage_.raw, storage_.size) = storage_.raw ? base::reallocate(storage_.raw, storage_.size, size) : base::allocate(size);
	}
public:
	using base::base;
	polymorphic_ptr () = default;
	polymorphic_ptr (polymorphic_ptr && other) = default;
	polymorphic_ptr & operator = (polymorphic_ptr && rhs) noexcept {
		deallocate();
		storage_ = std::move(rhs.storage);
		static_cast<base &>(*this) = std::move(rhs);
		return *this;
	}
	~polymorphic_ptr () noexcept(is_nothrow_destructible) {
		deallocate();
	}
	/**
	 *	Creates an object in the managed storage destroying
	 *	any object currently residing there and resizing the
	 *	managed storage if and only if necessary.
	 *
	 *	\tparam U
	 *		The type to create. Must derived from \em T.
	 *	\tparam Args
	 *		The types of arguments to forward through to a
	 *		constructor of \em U.
	 *
	 *	\param [in] args
	 *		Arguments to forward through to a constructor of
	 *		\em U.
	 *
	 *	\return
	 *		A reference to the newly-created object.
	 */
	template <typename U, typename... Args>
	U & emplace (Args &&... args) {
		reallocate<U>();
		return storage_.template emplace<U>(std::forward<Args>(args)...);
	}
	/**
	 *	Determines whether an object currently resides in
	 *	the managed storage.
	 *
	 *	\return
	 *		\em true if an object currently resides in the
	 *		managed storage, \em false otherwise.
	 */
	explicit operator bool () const noexcept {
		return bool(storage_.object);
	}
	/**
	 *	Retrieves a reference to the managed object downcast
	 *	to a reference to \em T.
	 *
	 *	\return
	 *		A reference.
	 */
	T & operator * () noexcept {
		return *storage_.object;
	}
	const T & operator * () const noexcept {
		return *storage_.object;
	}
	/**
	 *	Retrieves a pointer to the managed object downcast
	 *	to a pointer to \em T.
	 *
	 *	\return
	 *		A pointer.
	 */
	T * get () noexcept {
		return storage_.object;
	}
	const T * get () const noexcept {
		return storage_.object;
	}
	T * operator -> () noexcept {
		return get();
	}
	const T * operator -> () const noexcept {
		return get();
	}
	/**
	 *	Determines the size of the managed storage in bytes.
	 *
	 *	\return
	 *		The size.
	 */
	std::size_t capacity () const noexcept {
		return storage_.size;
	}
	/**
	 *	Destroys the managed object if there is a managed
	 *	object.
	 */
	void reset () noexcept(is_nothrow_destructible) {
		destroy();
	}
};

}
