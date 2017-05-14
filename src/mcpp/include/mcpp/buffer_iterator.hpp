/**
 *	\file
 */

#pragma once

#include "pointer_ops.hpp"
#include <cstddef>

namespace mcpp {

/**
 *	The type of an iterator to a raw buffer
 *	of bytes.
 *
 *	\tparam T
 *		The byte type which shall be yielded by
 *		dereferencing the iterator.
 */
template <typename T>
using buffer_iterator = T *;

/**
 *	Creates a \ref buffer_iterator which points
 *	to the beginning of a raw buffer of bytes.
 *
 *	\tparam T
 *		The type which shall be yielded by
 *		dereferencing the resulting iterator.
 *		Defaults to char.
 *
 *	\param [in] ptr
 *		A pointer to the beginning of the raw
 *		buffer of bytes.
 *
 *	\return
 *		A \ref buffer_iterator.
 */
template <typename T = char>
buffer_iterator<const T> make_buffer_iterator (const void * ptr) noexcept {
	return static_cast<buffer_iterator<const T>>(ptr);
}
template <typename T = char>
buffer_iterator<T> make_buffer_iterator (void * ptr) noexcept {
	return static_cast<buffer_iterator<T>>(ptr);
}
/**
 *	Creates a \ref buffer_iterator which points
 *	to the byte past the last byte in a raw buffer
 *	of bytes.
 *
 *	\tparam T
 *		The type which shall be yielded by
 *		dereferencing the resulting iterator.
 *		Defaults to char.
 *
 *	\param [in] ptr
 *		A pointer to the beginning of the raw
 *		buffer of bytes.
 *	\param [in] len
 *		The number of bytes in the buffer.
 *
 *	\return
 *		A \ref buffer_iterator.
 */
template <typename T = char>
buffer_iterator<const T> make_buffer_iterator (const void * ptr, std::size_t len) noexcept {
	return static_cast<buffer_iterator<const T>>(next(ptr, len));
}
template <typename T = char>
buffer_iterator<T> make_buffer_iterator (void * ptr, std::size_t len) noexcept {
	return static_cast<buffer_iterator<T>>(next(ptr, len));
}

}
