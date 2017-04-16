/**
 *	\file
 */

#pragma once

#include "traits.hpp"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/read.hpp>
#include <ios>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace iostreams {

/**
 *	A type which models `Source` by wrapping another
 *	`Source` and simply passing read operations through.
 *
 *	The purpose of this class is to create a wrapper
 *	which shields the wrapped `Source` from being closed,
 *	or to disambiguate the manner in which a `Device`
 *	is to be used (this is necessary for example when
 *	using `boost::iostreams::compose` with a `DualUseFilter`
 *	and a `Device` which is both a `Source` and `Sink`).
 *
 *	\tparam Source
 *		A type which models `Source`. Note that the
 *		proxy_source will hold an object of this type
 *		by value, if this is not desired use
 *		`boost::reference_wrapper` (note that it is important
 *		that you do not use `std::reference_wrapper` as the
 *		underlying Boost.IOStreams functionality is unaware
 *		of them).
 *
 *	\sa
 *		proxy_sink
 */
template <typename Source>
class proxy_source {
private:
	Source src_;
public:
	proxy_source () = delete;
	proxy_source (const proxy_source &) = default;
	proxy_source (proxy_source &&) = default;
	proxy_source & operator = (const proxy_source &) = default;
	proxy_source & operator = (proxy_source &&) = default;
	/**
	 *	Creates a new proxy_source.
	 *
	 *	\param [in] src
	 *		The `Source` to wrap.
	 */
	explicit proxy_source (Source src) noexcept(std::is_nothrow_move_constructible<Source>::value)
		:	src_(std::move(src))
	{	}
	using char_type = char_type_of_t<Source>;
	class category
		:	public boost::iostreams::device_tag,
			public boost::iostreams::input
	{	};
	std::streamsize read (char_type * s, std::streamsize n) {
		return boost::iostreams::read(src_, s, n);
	}
};

/**
 *	Creates and returns a \ref proxy_source.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *
 *	\param [in] src
 *		The `Source` to wrap.
 *
 *	\return
 *		A \ref proxy_source which wraps \em src.
 */
template <typename Source>
proxy_source<std::decay_t<Source>> make_proxy_source (Source && src) noexcept(
	std::is_nothrow_constructible<proxy_source<std::decay_t<Source>>, Source &&>::value
) {
	return proxy_source<std::decay_t<Source>>(std::forward<Source>(src));
}

}
}
