/**
 *	\file
 */

#pragma once

#include "traits.hpp"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/read.hpp>
#include <algorithm>
#include <cstddef>
#include <ios>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace iostreams {

/**
 *	A type which models `Source` by wrapping another
 *	`Source` and limiting the number of characters which
 *	may be read therefrom.
 *
 *	\tparam Source
 *		A type which models `Source`. Note that the
 *		limiting_source will hold an object of this type
 *		by value, if this is not desired use
 *		`boost::reference_wrapper` (note that it is important
 *		that you do not use `std::reference_wrapper` as the
 *		underlying Boost.IOStreams functionality is unaware
 *		of them).
 */
template <typename Source>
class limiting_source {
private:
	std::size_t limit_;
	Source src_;
public:
	limiting_source () = delete;
	limiting_source (const limiting_source &) = delete;
	limiting_source (limiting_source &&) = default;
	limiting_source & operator = (const limiting_source &) = delete;
	limiting_source & operator = (limiting_source &&) = default;
	/**
	 *	Creates a new limiting_source.
	 *
	 *	\param [in] src
	 *		The `Source` to wrap.
	 *	\param [in] limit
	 *		The number of characters which may be read
	 *		from \em src before EOF is reported.
	 */
	limiting_source (Source src, std::size_t limit) noexcept(std::is_nothrow_move_constructible<Source>::value)
		:	limit_(limit),
			src_(std::move(src))
	{	}
	class category
		:	public boost::iostreams::device_tag,
			public boost::iostreams::input
	{	};
	using char_type = char_type_of_t<Source>;
	std::streamsize read (char_type * s, std::streamsize n) {
		if (limit_ == 0) return -1;
		std::size_t to_read = std::min(limit_, std::size_t(n));
		std::streamsize retr = boost::iostreams::read(src_, s, std::streamsize(to_read));
		if (retr == -1) return retr;
		limit_ -= std::size_t(retr);
		return retr;
	}
};

/**
 *	Creates and returns a \ref limiting_source.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *
 *	\param [in] src
 *		The `Source` to wrap.
 *	\param [in] limit
 *		The number of characters which the
 *		\ref limiting_source shall allow to be
 *		read from \em src.
 *
 *	\return
 *		A \ref limiting_source which wraps \em src
 *		and limits it to \em limit characters.
 */
template <typename Source>
limiting_source<std::decay_t<Source>> make_limiting_source (Source && src, std::size_t limit) noexcept(
	std::is_nothrow_constructible<limiting_source<std::decay_t<Source>>, Source &&>::value
) {
	return limiting_source<std::decay_t<Source>>(std::forward<Source>(src), limit);
}

}
}
