/**
 *	\file
 */

#pragma once

#include "traits.hpp"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/write.hpp>
#include <ios>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace iostreams {

/**
 *	A type which models `Sink` by wrapping another
 *	`Sink` and simply passing write operations through.
 *
 *	The purpose of this class is to create a wrapper
 *	which shields the wrapped `Sink` from being closed,
 *	or to disambiguate the manner in which a `Device`
 *	is to be used (this is necessary for example when
 *	using `boost::iostreams::compose` with a `DualUseFilter`
 *	and a `Device` which is both a `Source` and `Sink`).
 *
 *	\tparam Sink
 *		A type which models `Sink`. Note that the
 *		proxy_sink will hold an object of this type
 *		by value, if this is not desired use
 *		`boost::reference_wrapper` (note that it is important
 *		that you do not use `std::reference_wrapper` as the
 *		underlying Boost.IOStreams functionality is unaware
 *		of them).
 *
 *	\sa
 *		proxy_sink
 */
template <typename Sink>
class proxy_sink {
private:
	Sink sink_;
public:
	proxy_sink () = default;
	proxy_sink (const proxy_sink &) = default;
	proxy_sink (proxy_sink &&) = default;
	proxy_sink & operator = (const proxy_sink &) = default;
	proxy_sink & operator = (proxy_sink &&) = default;
	/**
	 *	Creates a new proxy_sink.
	 *
	 *	\param [in] sink
	 *		The `Sink` to wrap.
	 */
	explicit proxy_sink (Sink sink) noexcept(std::is_nothrow_move_constructible<Sink>::value)
		:	sink_(std::move(sink))
	{	}
	class category
		:	public boost::iostreams::device_tag,
			public boost::iostreams::output
	{	};
	using char_type = char_type_of_t<Sink>;
	std::streamsize write (const char_type * s, std::streamsize n) {
		return boost::iostreams::write(sink_, s, n);
	}
};

/**
 *	Creates and returns a \ref proxy_sink.
 *
 *	\tparam Sink
 *		A type which models `Sink`.
 *
 *	\param [in] sink
 *		The `Sink` to wrap.
 *
 *	\return
 *		A \ref proxy_sink which wraps \em src.
 */
template <typename Sink>
proxy_sink<std::decay_t<Sink>> make_proxy_sink (Sink && sink) noexcept(
	std::is_nothrow_constructible<proxy_sink<std::decay_t<Sink>>, Sink &&>::value
) {
	return proxy_sink<std::decay_t<Sink>>(std::forward<Sink>(sink));
}

}
}
