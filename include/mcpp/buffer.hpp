/**
 *	\file
 */

#pragma once

#include "buffer_iterator.hpp"
#include "pointer_ops.hpp"
#include <cstddef>
#include <iterator>
#include <streambuf>
#include <string>

namespace mcpp {

/**
 *	Derives from std::basic_streambuf and wraps raw
 *	regions of memory.
 *
 *	Note that objects of this type adapt buffers of
 *	various types. Accordingly be aware of the C++
 *	strict aliasing rules when selecting \em CharT:
 *	In many cases signed char does not quality.
 *
 *	In all cases no copy is made of the regions of
 *	memory passed to this object and therefore
 *	those regions must remain valid so long as
 *	objects which own them may still be used for
 *	operations.
 *
 *	\tparam CharT
 *		The character type.
 *	\tparam Traits
 *		The traits type.
 */
template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_buffer final : public std::basic_streambuf<CharT, Traits> {
private:
	static CharT * to_pointer (void * ptr) noexcept {
		return static_cast<CharT *>(ptr);
	}
	static CharT * to_pointer (const void * ptr) noexcept {
		return to_pointer(const_cast<void *>(ptr));
	}
	using base = std::basic_streambuf<CharT, Traits>;
	void set (void * begin, void * end) noexcept {
		auto b = to_pointer(begin);
		auto e = to_pointer(end);
		base::setg(b, b, e);
		base::setp(b, e);
	}
	void set (const void * begin, const void * end) noexcept {
		auto b = to_pointer(begin);
		auto e = to_pointer(end);
		base::setg(b, b, e);
		base::setp(nullptr, nullptr);
	}
public:
	basic_buffer () = default;
	using base::base;
	/**
	 *	Creates a buffer from an array.
	 *
	 *	\tparam T
	 *		The type of the array.
	 *	\tparam N
	 *		The number of elements in the array.
	 *
	 *	\param [in] arr
	 *		A reference to the array.
	 */
	template <typename T, std::size_t N>
	basic_buffer (T (& arr) [N]) noexcept {
		assign(arr);
	}
	/**
	 *	Creates a buffer from a pointer and a length.
	 *
	 *	\tparam T
	 *		The type pointed to by \em ptr.
	 *
	 *	\param [in] ptr
	 *		A pointer to the first byte in the buffer.
	 *	\param [in] len
	 *		The number of bytes in the buffer pointed to
	 *		by \em ptr.
	 */
	template <typename T>
	basic_buffer (T * ptr, std::size_t len) noexcept {
		assign(ptr, len);
	}
	/**
	 *	Creates a buffer from a pointer to the beginning
	 *	and one past the end of a region of memory.
	 *
	 *	\tparam T
	 *		The type pointed to by \em begin and \em end.
	 *
	 *	\param [in] begin
	 *		A pointer to the first byte.
	 *	\param [in] end
	 *		A pointer to one past the last byte.
	 */
	template <typename T>
	basic_buffer (T * begin, T * end) noexcept {
		assign(begin, end);
	}
	/**
	 *	Replaces the underlying buffer with an array.
	 *
	 *	\tparam T
	 *		The type pointed to by \em ptr.
	 *
	 *	\param [in] ptr
	 *		A pointer to the first byte in the buffer.
	 *	\param [in] len
	 *		The number of bytes in the buffer pointed to
	 *		by \em ptr.
	 */
	template <typename T, std::size_t N>
	void assign (T (& arr) [N]) noexcept {
		using std::begin;
		using std::end;
		set(begin(arr), end(arr));
	}
	/**
	 *	Replaces the underlying buffer with an area of
	 *	memory.
	 *
	 *	\tparam T
	 *		The type of the array.
	 *	\tparam N
	 *		The number of elements in the array.
	 *
	 *	\param [in] arr
	 *		A reference to the array.
	 */
	template <typename T>
	void assign (T * ptr, std::size_t len) noexcept {
		set(ptr, make_buffer_iterator(ptr, len));
	}
	/**
	 *	Replaces the underlying buffer with a range.
	 *
	 *	\tparam T
	 *		The type pointed to by \em begin and \em end.
	 *
	 *	\param [in] begin
	 *		A pointer to the first byte.
	 *	\param [in] end
	 *		A pointer to one past the last byte.
	 */
	template <typename T>
	void assign (T * begin, T * end) noexcept {
		set(begin, end);
	}
	/**
	 *	Determines the number of bytes which have
	 *	been written to the buffer.
	 *
	 *	\return
	 *		The number of bytes.
	 */
	std::size_t written () const noexcept {
		return distance(base::pbase(), base::pptr());
	}
	/**
	 *	Determines the number of bytes which have
	 *	been read from the buffer.
	 *
	 *	\return
	 *		The number of bytes.
	 */
	std::size_t read () const noexcept {
		return distance(base::eback(), base::gptr());
	}
};

/**
 *	A \ref basic_buffer which uses default
 *	template parameters.
 */
using buffer = basic_buffer<char>;

}
