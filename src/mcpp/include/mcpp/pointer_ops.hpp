/**
 *	\file
 */

#pragma once

#include <cstddef>

namespace mcpp {

/**
 *	Transforms a pointer to a byte such that
 *	it points to the Nth byte thereafter.
 *
 *	\param [in] ptr
 *		A pointer to a byte.
 *	\param [in] n
 *		The zero relative index of the byte after
 *		\em ptr at which the result shall point.
 *		Defaults to 1.
 *
 *	\return
 *		\em ptr advanced by \em n bytes.
 */
inline const void * next (const void * ptr, std::size_t n = 1) noexcept {
	return static_cast<const char *>(ptr) + n;
}
inline void * next (void * ptr, std::size_t n = 1) noexcept {
	return static_cast<char *>(ptr) + n;
}

/**
 *	Calculates the number of bytes which separate
 *	two pointers.
 *
 *	It must be the case that \em begin and \em end
 *	point to the same area of memory, or one past
 *	the end of that area of memory, and that \em begin
 *	points to the same location or a location before
 *	\em end.
 *
 *	\param [in] begin
 *		The first pointer.
 *	\param [in] end
 *		The second pointer.
 *
 *	\return
 *		The distance in bytes between \em begin and
 *		\em end.
 */
inline std::size_t distance (const void * begin, const void * end) noexcept {
	return std::size_t(static_cast<const char *>(end) - static_cast<const char *>(begin));
}

}
