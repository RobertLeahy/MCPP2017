/**
 *	\file
 */

#pragma once

#include "traits.hpp"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/read.hpp>
#include <cstddef>
#include <ios>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace iostreams {

namespace detail {

template <typename...>
class concatenating_source_char_type;

template <typename Source, typename T, typename... Sources>
class concatenating_source_char_type<Source, T, Sources...> {
public:
	using type = char_type_of_t<Source>;
	static_assert(
		std::is_same<type, typename concatenating_source_char_type<T, Sources...>::type>::value,
		"All Sources must have the same character type"
	);
};

template <typename Source>
class concatenating_source_char_type<Source> {
public:
	using type = char_type_of_t<Source>;
};

template <>
class concatenating_source_char_type<> {
public:
	using type = char;
};

}

/**
 *	A type which models `Source` by wrapping a number
 *	of objects which model `Source` and providing a
 *	character sequence which is the concatenation of
 *	the character sequences of those managed models
 *	of `Source`.
 *
 *	A concatenating_source is a model of `Closable`.
 *	Closing a concatenating_source shall evaluate the
 *	expression `boost::iostreams::close(src, std::ios_base::in)`
 *	with `src` substituted for each wrapped `Source`.
 *
 *	This template may be instantiated with no template
 *	arguments. An instance of such a class has a character
 *	type of `char`, unconditionally reports end of stream
 *	in response to attempts to read from it, and does
 *	nothing when closed.
 *
 *	\tparam Sources
 *		A number of models of `Source` which share a
 *		character type. If they do not share a character
 *		type the behavior is undefined. Note that these
 *		shall be held by value, if this is not desired
 *		use `boost::reference_wrapper` (note that due
 *		to the underlying Boost.IOStreams functionality
 *		being unaware of `std::reference_wrapper` this
 *		will not suffice).
 */
template <typename... Sources>
class concatenating_source {
private:
	using tuple_type = std::tuple<Sources...>;
	tuple_type srcs_;
public:
	concatenating_source (const concatenating_source &) = default;
	concatenating_source (concatenating_source &&) = default;
	concatenating_source & operator = (const concatenating_source &) = default;
	concatenating_source & operator = (concatenating_source &&) = default;
	/**
	 *	Creates a concatenating_source which wraps
	 *	certain models of `Source`.
	 *
	 *	\param [in] srcs
	 *		An instance of each of \em Sources. Shall be
	 *		moved into the newly-created object.
	 */
	concatenating_source (Sources... srcs) noexcept(
		std::is_nothrow_constructible<tuple_type, Sources &&...>::value
	)	:	srcs_(std::forward<Sources>(srcs)...)
	{	}
	class category
		:	public boost::iostreams::closable_tag,
			public boost::iostreams::device_tag,
			public boost::iostreams::input
	{	};
	using char_type = typename detail::concatenating_source_char_type<Sources...>::type;
private:
	template <std::size_t I>
	using tag_type = std::integral_constant<bool, I == sizeof...(Sources)>;
	template <std::size_t I>
	std::streamsize read_impl (char_type * s, std::streamsize n, const std::false_type &) {
		if (n == 0) return 0;
		auto && src = std::get<I>(srcs_);
		auto retr = boost::iostreams::read(src, s, n);
		tag_type<I + 1> tag;
		if (retr == -1) return read_impl<I + 1>(s, n, tag);
		n -= retr;
		s += retr;
		auto next = read_impl<I + 1>(s, n, tag);
		if (next == -1) return retr;
		return retr + next;
	}
	template <std::size_t>
	constexpr std::streamsize read_impl (const char_type *, std::streamsize, const std::true_type &) {
		return -1;
	}
	template <std::size_t I>
	void close_impl (const std::false_type &) {
		boost::iostreams::close(std::get<I>(srcs_), std::ios_base::in);
		tag_type<I + 1> tag;
		close_impl<I + 1>(tag);
	}
	template <std::size_t>
	void close_impl (const std::true_type &) {	}
public:
	std::streamsize read (char_type * s, std::streamsize n) {
		tag_type<0> tag;
		return read_impl<0>(s, n, tag);
	}
	void close () {
		tag_type<0> tag;
		close_impl<0>(tag);
	}
};

/**
 *	Creates a \ref concatenating_source.
 *
 *	\tparam Sources
 *		The types which model `Source` that the resulting
 *		\ref concatenating_source shall wrap.
 *
 *	\param [in] srcs
 *		The models of `Source` which shall be wrapped
 *		by the result.
 *
 *	\return
 *		A \ref concatenating_source which wraps \em srcs.
 */
template <typename... Sources>
concatenating_source<std::decay_t<Sources>...> make_concatenating_source (Sources &&... srcs) noexcept(
	std::is_nothrow_constructible<concatenating_source<std::decay_t<Sources>...>, Sources &&...>::value &&
	std::is_nothrow_move_constructible<concatenating_source<std::decay_t<Sources>...>>::value
) {
	return concatenating_source<std::decay_t<Sources>...>(std::forward<Sources>(srcs)...);
}

}
}
