/**
 *	\file
 */

#pragma once

#include "error.hpp"
#include "varint.hpp"
#include <boost/core/ref.hpp>
#include <boost/expected/expected.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/tee.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/iostreams/concatenating_source.hpp>
#include <mcpp/optional.hpp>
#include <cassert>
#include <cstddef>
#include <system_error>
#include <utility>

namespace mcpp {
namespace protocol {

namespace detail {

template <typename T, typename CharT>
class incremental_varint_parser_base {
static_assert(sizeof(CharT) == 1, "Varint serialization is byte oriented");
private:
	static constexpr auto size = varint_size<T>;
	CharT buffer_ [size];
	using buffer_type = basic_buffer<CharT>;
	buffer_type out_;
protected:
	optional<T> res_;
	template <typename Source>
	using source_type = iostreams::concatenating_source<
		buffer_type,
		boost::iostreams::composite<
			boost::iostreams::tee_filter<
				boost::reference_wrapper<buffer_type>
			>,
			boost::reference_wrapper<Source>
		>
	>;
	template <typename Source>
	source_type<Source> make_source (Source & src) {
		return iostreams::make_concatenating_source(
			buffer_type(buffer_, out_.written()),
			boost::iostreams::compose(
				boost::iostreams::tee(boost::ref(out_)),
				boost::ref(src)
			)
		);
	}
public:
	incremental_varint_parser_base () : out_(buffer_) {	}
	incremental_varint_parser_base (const incremental_varint_parser_base &) = delete;
	incremental_varint_parser_base (incremental_varint_parser_base &&) = delete;
	incremental_varint_parser_base & operator = (const incremental_varint_parser_base &) = delete;
	incremental_varint_parser_base & operator = (incremental_varint_parser_base &&) = delete;
	using char_type = CharT;
	using value_type = T;
	using parse_result_type = boost::expected<optional<T>, std::error_code>;
	/**
	 *	Resets the internal state clearing all cached bytes
	 *	and any result.
	 */
	void reset () noexcept {
		out_.assign(buffer_);
		res_ = nullopt;
	}
	/**
	 *	Retrieves the number of characters cached
	 *	by the parser. After a Varint has successfully
	 *	been parsed this shall return the length of
	 *	the representation thereof until \ref reset
	 *	is called.
	 *
	 *	\return
	 *		The number of characters cached.
	 */
	std::size_t cached () const noexcept {
		return out_.written();
	}
	/**
	 *	Determines whether or not this parser has
	 *	cached any characters.
	 *
	 *	Equivalent to `cached() == 0`.
	 *
	 *	\return
	 *		\em true if this parser has no cached
	 *		characters, \em false otherwise.
	 */
	bool empty () const noexcept {
		return cached() == 0;
	}
	/**
	 *	Retrieves the parsed value.
	 *
	 *	If this object does not contain a parsed
	 *	value the behavior is undefined.
	 */
	 T get () const noexcept {
		 assert(res_);
		 return *res_;
	 }
};

}

/**
 *	Incrementally parses a Varint.
 *
 *	As opposed to \ref parse_varint which either
 *	parses a Varint or returns \ref error::end_of_file
 *	if there are insufficient bytes this class will
 *	retain consumed bytes between calls to \ref parse
 *	and reuse them as more bytes are made available
 *	until an entire Varint is parsed.
 *
 *	\tparam T
 *		The type of integer to parse. Must be an integer
 *		type which may be parsed by \ref parse_varint.
 *	\tparam CharT
 *		The character type of the `Source`s which shall be
 *		used for parsing. Note that the type of the `Source`
 *		used may vary between calls to \ref parse however
 *		the underlying character type must remain the same.
 *		Defaults to `char`.
 */
template <typename T, typename CharT = char>
class incremental_varint_parser : public detail::incremental_varint_parser_base<T, CharT> {
private:
	using base = detail::incremental_varint_parser_base<T, CharT>;
public:
	/**
	 *	Attempts to parse a Varint, caching consumed
	 *	bytes for the next call if that is not possible.
	 *
	 *	Once an integer is returned that integer will be
	 *	returned unconditionally and no further bytes
	 *	will be consumed until \ref reset is invoked.
	 *
	 *	\tparam Source
	 *		The type of `Source` from which bytes shall
	 *		be drawn to form a Varint.
	 *
	 *	\param [in] src
	 *		The `Source` from which bytes shall be drawn.
	 *
	 *	\return
	 *		No value if the parse is incomplete. A value of
	 *		type \em T if the parse completes or is complete.
	 *		A `std::error_code` if there is an error.
	 */
	template <typename Source>
	typename base::parse_result_type parse (Source & src) {
		if (base::res_) return base::res_;
		auto impl = base::make_source(src);
		auto res = protocol::parse_varint<T>(impl);
		if (res) return base::res_ = make_optional(*res);
		if (res.error() == make_error_code(error::end_of_file)) return optional<T>{};
		return res.get_unexpected();
	}
};

}
}
