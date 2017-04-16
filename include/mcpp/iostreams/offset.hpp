/**
 *	\file
 */

#pragma once

#include "../optional.hpp"
#include "traits.hpp"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/seek.hpp>
#include <cstddef>
#include <ios>
#include <type_traits>

namespace mcpp {
namespace iostreams {

namespace detail {

template <typename Device>
optional<std::size_t> offset (Device & device, std::ios_base::openmode which, const std::true_type &) {
	auto pos = boost::iostreams::seek(device, 0, std::ios_base::cur, which);
	if (pos == -1) return nullopt;
	return std::size_t(pos);
}
template <typename Device>
constexpr optional<std::size_t> offset (Device &, std::ios_base::openmode, const std::false_type &) {
	return nullopt;
}

}

/**
 *	Attemps to obtain the current offset for some
 *	object which models `Device`.
 *
 *	\tparam Device
 *		Some type which models `Device`.
 *
 *	\param [in] device
 *		An object which models `Device` whose offset
 *		shall be obtained.
 *	\param [in] which
 *		`std::ios_base::in` to obtain the offset of
 *		the input head of \em device, `std::ios_base::out`
 *		to obtain the offset of the output head of
 *		\em device, the bitwise or of these constants
 *		to obtain the offset of both heads (assumes both
 *		heads have the same offset).
 *
 *	\return
 *		The offset if it could be obtained.
 */
template <typename Device>
optional<std::size_t> offset (Device & device, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
	in_any_category_t<Device,
		boost::iostreams::istream_tag,
		boost::iostreams::ostream_tag,
		boost::iostreams::streambuf_tag,
		boost::iostreams::input_seekable,
		boost::iostreams::output_seekable,
		boost::iostreams::dual_seekable,
		boost::iostreams::bidirectional_seekable,
		boost::iostreams::seekable
	> tag;
	return detail::offset(device, which, tag);
}

}
}
