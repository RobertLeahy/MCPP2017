/**
 *	\file
 */

#pragma once

#include "checked.hpp"
#include "error.hpp"
#include "exception.hpp"
#include "varint.hpp"
#include <boost/core/ref.hpp>
#include <boost/expected/expected.hpp>
#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/write.hpp>
#include <mcpp/checked.hpp>
#include <mcpp/iostreams/limiting_source.hpp>
#include <mcpp/iostreams/proxy_sink.hpp>
#include <mcpp/iostreams/traits.hpp>
#include <codecvt>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <ios>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace protocol {

namespace detail {

class default_codecvt {	};

template <typename InternT, typename ExternT>
class select_codecvt {
public:
	using type = std::codecvt<InternT, ExternT, std::mbstate_t>;
};
template <>
class select_codecvt<char16_t, char> {
public:
	using type = std::codecvt_utf8_utf16<char16_t>;
};
template <>
class select_codecvt<char32_t, char> {
public:
	using type = std::codecvt_utf8<char32_t>;
};

template <typename InternT, typename ExternT>
using select_codecvt_t = typename select_codecvt<InternT, ExternT>::type;

template <typename CharT, typename Codecvt, typename Device>
using codecvt_t = std::conditional_t<std::is_same<Codecvt, default_codecvt>::value,
	select_codecvt_t<CharT, iostreams::char_type_of_t<Device>>,
	Codecvt
>;

template <typename Codecvt, typename Device, typename Allocator>
boost::iostreams::code_converter<Device, Codecvt, Allocator> make_code_converter (const Device & device, const Allocator &, const std::false_type &) {
	//	TODO: Use the allocator?
	return boost::iostreams::code_converter<std::decay_t<Device>, Codecvt, Allocator>(device);
}
template <typename, typename Device, typename Allocator>
typename boost::unwrap_reference<Device>::type & make_code_converter (Device & device, const Allocator &, const std::true_type &) {
	return boost::unwrap_ref(device);
}
template <typename CharT, typename Codecvt, typename Device, typename Allocator>
decltype(auto) make_code_converter (Device & device, const Allocator & a) {
	typename std::is_same<CharT, iostreams::char_type_of_t<Device>>::type tag;
	return detail::make_code_converter<codecvt_t<CharT, Codecvt, Device>>(device, a, tag);
}

}

/**
 *	Parses a Unicode string from a binary buffer.
 *
 *	\tparam CharT
 *		The character type of the resulting string.
 *	\tparam Traits
 *		The character traits type of the resulting
 *		string. Defaults to `std::char_traits<CharT>`.
 *	\tparam Codecvt
 *		The `std::codecvt` facet to use to convert
 *		the parsed text. In the case where @em CharT
 *		is a byte type conversion is not necessary
 *		and the default template parameter will cause
 *		the function to not perform conversion. In
 *		the case where @em CharT is `wchar_t`, `char16_t`,
 *		or `char32_t` conversion is necessary and will
 *		be performed to UTF-16 or UTF-32 as appropriate
 *		based no the width of @em CharT.
 *	\tparam Allocator
 *		A type which models `Allocator`.
 *	\tparam Source
 *		A type which models `Source`.
 *
 *	\param [in] src
 *		The `Source` from which data shall be read.
 *	\param [in] a
 *		The `Allocator` to use. Defaults to a default
 *		constructed object of type \em Allocator.
 *
 *	\return
 *		A string if the parse succeeds. Otherwise a
 *		`std::error_code` object encapsulating the cause of
 *		the failure. Note that failures from the conversion
 *		process are transmitted as exceptions not
 *		`std::error_code` objects.
 */
template <typename CharT = char, typename Traits = std::char_traits<CharT>, typename Codecvt = detail::default_codecvt, typename Allocator = std::allocator<CharT>, typename Source>
boost::expected<std::basic_string<CharT, Traits, Allocator>, std::error_code> parse_string (Source & src, const Allocator & a = Allocator{}) {
	using string = std::basic_string<CharT, Traits, Allocator>;
	using stringbuf = std::basic_stringbuf<CharT, Traits, Allocator>;
	using result = boost::expected<string, std::error_code>;
	return protocol::parse_varint<std::uint32_t>(src).bind([&] (auto num) {
		return checked::cast<std::size_t>(num).bind([&] (auto size) -> result {
			auto limiting = iostreams::make_limiting_source(boost::ref(src), size);
			auto && converting = detail::make_code_converter<CharT, Codecvt>(boost::ref(limiting), a);
			string s(a);
			stringbuf buf(s, std::ios_base::out);
			std::size_t num(boost::iostreams::copy(boost::ref(converting), buf));
			if (num != size) return boost::make_unexpected(make_error_code(error::end_of_file));
			return buf.str();
		});
	});
}
/**
 *	Functions identically to \ref parse_string except
 *	the result of the parse shall be assigned to an out
 *	parameter rather than being returned.
 *
 *	This externalizes error handling and also allows
 *	template argument deduction to deduce \em CharT and
 *	\em Traits rather than forcing the consumer to provide
 *	them explicitly if they wish to customize them.
 *
 *	\tparam Codecvt
 *		See \ref parse_string.
 *	\tparam Source
 *		A model of `Source`.
 *	\tparam CharT
 *		The character type of the string to parse.
 *	\tparam Traits
 *		The traits type of the string to parse.
 *	\tparam Allocator
 *		A model of `Allocator` which is used to allocate
 *		memory for the string to parse.
 *
 *	\param [in] src
 *		The `Source` from which the representation of the
 *		string shall be read.
 *	\param [out] val
 *		A string which shall be assigned the result of the
 *		parse. If the parse fails the value of this string
 *		is unspecified except that it shall be safe to
 *		destroy and assign to.
 *
 *	\return
 *		Nothing on success. A `std::error_code` on failure.
 */
template <typename Codecvt = detail::default_codecvt, typename Source, typename CharT, typename Traits, typename Allocator>
boost::expected<void, std::error_code> parse_string (Source & src, std::basic_string<CharT, Traits, Allocator> & val) {
	return protocol::parse_string<CharT, Traits, Codecvt>(src, val.get_allocator()).map(
		[&] (auto && str) {	val = std::move(str);	}
	);
}

/**
 *	Creates a function which when invoked parses a
 *	string from a `Source`, assigns the parsed value
 *	(if any) to a provided string, and returns a
 *	`boost::expected<void, std::error_code` encapsulating
 *	the result of the operation.
 *
 *	This function is provided to make it convenient to
 *	chain parse operations together using `boost::expected::bind`
 *	and `boost::expected::map`.
 *
 *	\tparam Codecvt
 *		See \ref parse_string.
 *	\tparam Source
 *		A model of `Source`.
 *	\tparam CharT
 *		The character type of the string to parse.
 *	\tparam Traits
 *		The traits type of the string to parse.
 *	\tparam Allocator
 *		A model of `Allocator` which is used to allocate
 *		memory for the string to parse.
 *
 *	\param [in] src
 *		The `Source` from which a string shall be parsed when
 *		the resulting function is invoked. This reference must
 *		remain valid so long as the resulting function is
 *		liable to be invoked or the behavior is undefined.
 *	\param [in] str
 *		The string to which the result of the parse shall be
 *		assigned. If any parse fails the value of this string
 *		is unspecified except that it shall be safe to destroy
 *		and assign. This reference must remain valid so long as
 *		the resulting function is liable to be invoked or the
 *		behavior is undefined.
 *
 *	\return
 *		A function which behaves as described.
 */
template <typename Codecvt = detail::default_codecvt, typename Source, typename CharT, typename Traits, typename Allocator>
auto make_string_parser (Source & src, std::basic_string<CharT, Traits, Allocator> & str) noexcept {
	return [&] () {	return protocol::parse_string<Codecvt>(src, str);	};
}

namespace detail {

template <typename SizeType, typename Sink>
void serialize_string_size (SizeType size, Sink & sink) {
	auto s = mcpp::checked::cast<std::uint32_t>(size);
	if (!s) {
		std::ostringstream ss;
		ss << "Could not represent string length " << size;
		throw unrepresentable_error(ss.str());
	}
	protocol::serialize_varint(*s, sink);
}

template <typename Traits, typename Allocator, typename Sink>
void serialize_string (const std::basic_string<iostreams::char_type_of_t<Sink>, Traits, Allocator> & val, Sink & sink) {
	auto str_size = val.size();
	detail::serialize_string_size(str_size, sink);
	auto ptr = reinterpret_cast<const iostreams::char_type_of_t<Sink> *>(val.data());
	std::size_t written(boost::iostreams::write(sink, ptr, std::streamsize(str_size)));
	if (written != str_size) throw write_overflow_error(str_size, written);
}

template <typename Codecvt, typename CharT, typename Traits, typename Allocator, typename Sink>
void serialize_string (const std::basic_string<CharT, Traits, Allocator> & val, Sink & sink, const std::true_type &) {
	detail::serialize_string(val, sink);
}

template <typename Codecvt, typename CharT, typename Traits, typename Allocator, typename Sink>
void serialize_string (const std::basic_string<CharT, Traits, Allocator> & val, Sink & sink, const std::false_type &) {
	using char_type = iostreams::char_type_of_t<Sink>;
	using traits = std::char_traits<char_type>;
	using string = std::basic_string<char_type, traits, Allocator>;
	using stringbuf = std::basic_stringbuf<char_type, traits, Allocator>;
	string str(val.get_allocator());
	stringbuf buf(str, std::ios_base::out);
	//	Not sure why this proxy is necessary, if I don't
	//	use it I get a bunch of compile time errors I suspect
	//	because the boost::iostreams::code_converter isn't
	//	writable when templated on basic_stringbuf
	auto proxy = iostreams::make_proxy_sink(boost::ref(buf));
	//	TODO: Use the allocator?
	boost::iostreams::code_converter<boost::reference_wrapper<decltype(proxy)>, Codecvt, Allocator> converter(boost::ref(proxy));
	auto str_size = val.size();
	std::size_t written(boost::iostreams::write(converter, val.data(), std::streamsize(str_size)));
	if (written != str_size) throw write_overflow_error(str_size, written);
	//	Calling boost::iostreams::close isn't sufficient,
	//	neither is just calling converter.close(). This is
	//	because boost::iostreams::code_converter::close has
	//	its parameter defaulted to std::ios_base::out |
	//	std::ios_base::in but internally checks that parameter
	//	directly using == (both of these checks are clearly
	//	false when the parameter is a bitwise combination).
	//
	//	Calling just converter.close(std::ios_base::out) (which
	//	is all we care about) causes the destructor to crash.
	//	Calling converter.close(std::ios_base::in) after
	//	converter.close(std::ios_base::out) has the same effect.
	//	Both of these are caused by the fact that
	//	converter.close(std::ios_base::out) destroys a boost::
	//	optional the boost::iostreams::code_converter maintains
	//	internally whereas converter.close(std::ios_base::in)
	//	blindly accesses this optional (which fails with an assert
	//	in debug mode).
	//
	//	None of this is documented and I suspect it's two
	//	separate bugs.
	//
	//	This was discovered with Boost 1.61.0 specifically.
	converter.close(std::ios_base::in);
	converter.close(std::ios_base::out);
	detail::serialize_string(buf.str(), sink);
}

}

/**
 *	Serializes a Unicode string to a binary buffer.
 *
 *	\tparam Codecvt
 *		The `std::codecvt` facet to use to convert
 *		the text to serialize. No conversion will be
 *		performed in the case where \em CharT and
 *		the character type of \em Sink are identical.
 *		By default an appropriate `std::codecvt` facet
 *		is chosen based on the type of \em CharT and
 *		the character type of \em Sink.
 *	\tparam CharT
 *		The character type of the string.
 *	\tparam Traits
 *		The character traits type of the string.
 *	\tparam Allocator
 *		A type which models `Allocator`.
 *	\tparam Sink
 *		A type which models `Sink`.
 *
 *	\param [in] val
 *		The string to serialize. The result of invoking
 *		`get_allocator` on this object shall be used
 *		to obtain an allocator for all other memory
 *		allocating functionality of this function.
 *	\param [in] sink
 *		The `Sink` to which the serialization of \em val
 *		shall be written.
 */
template <typename Codecvt = detail::default_codecvt, typename CharT, typename Traits, typename Allocator, typename Sink>
void serialize_string (const std::basic_string<CharT, Traits, Allocator> & val, Sink & sink) {
	typename std::is_same<iostreams::char_type_of_t<Sink>, CharT>::type tag;
	detail::serialize_string<detail::codecvt_t<CharT, Codecvt, Sink>>(val, sink, tag);
}

}
}
