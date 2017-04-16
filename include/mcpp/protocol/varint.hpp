/**
 *	\file
 */

#pragma once

#include "../iostreams/traits.hpp"
#include "error.hpp"
#include "exception.hpp"
#include <boost/expected/expected.hpp>
#include <boost/iostreams/get.hpp>
#include <boost/iostreams/write.hpp>
#include <cstddef>
#include <cstring>
#include <ios>
#include <type_traits>
#include <system_error>

namespace mcpp {
namespace protocol {

namespace detail {

template <typename T>
constexpr std::size_t number_of_bits = sizeof(T) * 8;
constexpr std::size_t varint_bits_per_byte = 7;
template <typename T>
constexpr std::size_t varint_whole_bytes = number_of_bits<T> / varint_bits_per_byte;
template <typename T>
constexpr std::size_t varint_remaining_bits = number_of_bits<T> % varint_bits_per_byte;
template <typename T>
constexpr std::size_t varint_bits_in_last_byte = (varint_remaining_bits<T> == 0) ? 7 : varint_remaining_bits<T>;
template <typename T>
constexpr std::size_t varint_partial_byte = (varint_bits_in_last_byte<T> == 7) ? 0 : 1;
template <typename T>
constexpr std::size_t varint_bits_in_overflow_mask = 7 - varint_bits_in_last_byte<T>;
template <typename T, std::size_t N>
constexpr T varint_build_overflow_mask = (varint_build_overflow_mask<T, N - 1> >> 1) | 64;
template <typename T>
constexpr T varint_build_overflow_mask<T, 0> = 0;
template <typename T>
constexpr std::make_unsigned_t<T> varint_overflow_mask = varint_build_overflow_mask<std::make_unsigned_t<T>, varint_bits_in_overflow_mask<T>>;

}

/**
 *	The number of bytes it will take at maximum
 *	to encode an integer of type \em T as a varint.
 *
 *	Since varint is a variable width encoding and
 *	since the minimum size of a varint representation
 *	is 1 it doesn't make sense to encode a byte or
 *	character as a varint. Accordingly if @em T is
 *	such an integer type the value is undefined.
 *
 *	\tparam T
 *		The integer type.
 */
template <typename T>
constexpr std::size_t varint_size = detail::varint_whole_bytes<T> + detail::varint_partial_byte<T>;

namespace detail {

template <typename T, typename Source>
boost::expected<std::make_unsigned_t<T>, std::error_code> parse_varint_raw (Source & src) {
	constexpr std::size_t max = varint_size<T>;
	using type = std::make_unsigned_t<T>;
	type retr(0);
	for (std::size_t i = 0; i < max; ++i) {
		auto in = boost::iostreams::get(src);
		using traits_type = iostreams::traits_of_t<Source>;
		if (in == traits_type::eof()) {
			return boost::make_unexpected(make_error_code(error::end_of_file));
		}
		unsigned char curr(traits_type::to_char_type(in));
		type val(curr);
		val &= 127;
		//	Check for overflow on final byte
		if ((i == (max - 1)) && (val & varint_overflow_mask<T>)) {
			return boost::make_unexpected(make_error_code(error::unrepresentable));
		}
		retr |= val << (varint_bits_per_byte * i);
		if (curr == val) {
			auto prev = i;
			++i;
			//	Reject overlong encodings
			if ((prev != 0) && (curr == 0)) {
				return boost::make_unexpected(make_error_code(error::overlong));
			}
			return retr;
		}
	}
	return boost::make_unexpected(make_error_code(error::unrepresentable));
}

template <typename T, typename Source>
boost::expected<T, std::error_code> parse_varint (Source & src, const std::true_type &) {
	return detail::parse_varint_raw<T>(src).map([] (auto i) noexcept {
		T retr;
		//	Assumption: This machine represents
		//	signed numbers using two's complement
		std::memcpy(&retr, &i, sizeof(retr));
		return retr;
	});
}
template <typename T, typename Source>
boost::expected<T, std::error_code> parse_varint (Source & src, const std::false_type &) {
	return detail::parse_varint_raw<T>(src);
}

template <typename T>
void zigzag () noexcept {
	static_assert(std::is_signed<T>::value, "ZigZag encoding is for signed types only");
}

template <typename T>
std::make_signed_t<T> from_zigzag (T val) noexcept {
	std::make_signed_t<T> retr(val / 2);
	if ((val % 2) == 0) return retr;
	retr = -retr;
	return retr - 1;
}

}

/**
 *	Parses a varint from a `Source`.
 *
 *	\tparam T
 *		The type of integer to parse.
 *	\tparam Source
 *		A type which models `Source`.
 *
 *	\param [in] src
 *		The `Source` from which to read.
 *
 *	\return
 *		An integer of type \em T if a varint
 *		representation could be read, otherwise
 *		a `std::error_code` representing the error.
 */
template <typename T, typename Source>
boost::expected<T, std::error_code> parse_varint (Source & src) {
	typename std::is_signed<T>::type tag;
	return detail::parse_varint<T>(src, tag);
}
/**
 *	Functions identically to \ref parse_varint except
 *	the result of the parse shall be assigned to a variable
 *	passed by reference rather than being transmitted through
 *	the function's return value.
 *
 *	This externalizes error handling and also allows template
 *	argument deduction to deduce \em T rather than forcing
 *	the consumer to provide it explicitly.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *	\tparam T
 *		The type of integer to parse and assign to.
 *
 *	\param [in] src
 *		The `Source` from which to read.
 *	\param [out] val
 *		The variable to which the parse result shall be assigned.
 *		If the parse fails the value of this variable is unspecified.
 *
 *	\return
 *		Nothing on success. A `std::error_code` on failure.
 */
template <typename Source, typename T>
boost::expected<void, std::error_code> parse_varint (Source & src, T & val) {
	return protocol::parse_varint<T>(src).map([&] (auto i) noexcept {	val = i;	});
}
/**
 *	Parses a signed varint from a `Source` using
 *	ZigZag encoding.
 *
 *	\tparam T
 *		The type of integer to parse.
 *	\tparam Source
 *		A type which models `Source`.
 *
 *	\param [in] src
 *		The `Source` from which to read.
 *
 *	\return
 *		An integer of type \em T if a varint
 *		representation could be read, otherwise
 *		a `std::error_code` representing the error.
 */
template <typename T, typename Source>
boost::expected<T, std::error_code> parse_varint_zigzag (Source & src) {
	detail::zigzag<T>();
	return protocol::parse_varint<std::make_unsigned_t<T>>(src).map([] (auto u) noexcept {
		return detail::from_zigzag(u);
	});
}
/**
 *	Functions identically to \ref parse_varint_zigzag except
 *	the result of the parse shall be assigned to a variable
 *	passed by reference rather than being transmitted through
 *	the function's return value.
 *
 *	This externalizes error handling and also allows template
 *	argument deduction to deduce \em T rather than forcing
 *	the consumer to provide it explicitly.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *	\tparam T
 *		The type of integer to parse and assign to.
 *
 *	\param [in] src
 *		The `Source` from which to read.
 *	\param [out] val
 *		The variable to which the parse result shall be assigned.
 *		If the parse fails the value of this variable is unspecified.
 *
 *	\return
 *		Nothing on success. A `std::error_code` on failure.
 */
template <typename Source, typename T>
boost::expected<void, std::error_code> parse_varint_zigzag (Source & src, T & val) {
	return protocol::parse_varint_zigzag<T>(src).map([&] (auto i) noexcept {	val = i;	});
}

/**
 *	Creates a function which performs the following operations:
 *
 *	1.	Attempts to parse a varint from a `Source`
 *	2.	If that succeeds assigns the result to a `T`
 *	3.	Returns a `boost::expected<void, std::error_code>` which
 *		encapsulates the result of the parse
 *
 *	This function makes it easy to perform successive parses,
 *	stopping at the first failure, using `boost::expected::bind` and
 *	`boost::expected::map`.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *	\tparam T
 *		The type of integer which the created function will parse
 *		and assign.
 *
 *	\param [in] src
 *		The `Source` from which the varint shall be read. This
 *		reference must remain valid as long as the resulting function
 *		is liable to be invoked or the behavior is undefined.
 *	\param [in] val
 *		A variable which shall be assigned the value (if applicable)
 *		of each parse. If a parse fails the value is unspecified.
 *		This reference must remain valid as long as the resulting
 *		function is liable to be invoked or the behavior is undefined.
 *
 *	\return
 *		A function as described.
 */
template <typename Source, typename T>
auto make_varint_parser (Source & src, T & val) noexcept {
	return [&] () {	return protocol::parse_varint(src, val);	};
}
/**
 *	Creates a function which performs the following operations:
 *
 *	1.	Attempts to parse a ZigZag encoded varint from a `Source`
 *	2.	If that succeeds assigns the result to a `T`
 *	3.	Returns a `boost::expected<void, std::error_code>` which
 *		encapsulates the result of the parse
 *
 *	This function makes it easy to perform successive parses,
 *	stopping at the first failure, using `boost::expected::bind` and
 *	`boost::expected::map`.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *	\tparam T
 *		The type of integer which the created function will parse
 *		and assign.
 *
 *	\param [in] src
 *		The `Source` from which the varint shall be read. This
 *		reference must remain valid as long as the resulting function
 *		is liable to be invoked or the behavior is undefined.
 *	\param [in] val
 *		A variable which shall be assigned the value (if applicable)
 *		of each parse. If a parse fails the value is unspecified.
 *		This reference must remain valid as long as the resulting
 *		function is liable to be invoked or the behavior is undefined.
 *
 *	\return
 *		A function as described.
 */
template <typename Source, typename T>
auto make_varint_zigzag_parser (Source & src, T & val) noexcept {
	return [&] () {	return protocol::parse_varint_zigzag(src, val);	};
}

namespace detail {

template <typename T, typename Sink>
void serialize_varint_raw (T val, Sink & sink) {
	//	We create a buffer first to reduce
	//	the number of calls we need to make
	//	to the streambuf
	unsigned char buffer [varint_size<T>];
	std::size_t i = 0;
	for (;;) {
		buffer[i] = val & 127;
		val >>= varint_bits_per_byte;
		if (val != 0) {
			buffer[i++] |= 128;
			continue;
		}
		++i;
		break;
	}
	using pointer_type = const iostreams::char_type_of_t<Sink> *;
	auto ptr = reinterpret_cast<pointer_type>(buffer);
	std::size_t written(boost::iostreams::write(sink, ptr, std::streamsize(i)));
	if (written != i) throw write_overflow_error(i, written);
}

template <typename T, typename Sink>
void serialize_varint (T val, Sink & sink, const std::true_type &) {
	std::make_unsigned_t<T> u(val);
	detail::serialize_varint_raw(u, sink);
}
template <typename T, typename Sink>
void serialize_varint (T val, Sink & sink, const std::false_type &) {
	detail::serialize_varint_raw(val, sink);
}

template <typename T>
std::make_unsigned_t<T> to_zigzag (T val) noexcept {
	if (val < 0) {
		val += 1;
		val = -val;
		val *= 2;
		val += 1;
	} else {
		val *= 2;
	}
	return std::make_unsigned_t<T>(val);
}

}

/**
 *	Serializes an integer encoded as a varint.
 *
 *	\tparam T
 *		The type of integer to serialize.
 *	\tparam Sink
 *		A type which models `Sink`.
 *
 *	\param [in] val
 *		The integer to serialize.
 *	\param [in] sink
 *		An object which models `Sink` to which
 *		binary data shall be written.
 */
template <typename T, typename Sink>
void serialize_varint (T val, Sink & sink) {
	typename std::is_signed<T>::type tag;
	detail::serialize_varint(val, sink, tag);
}
/**
 *	Serializes a signed integer encoded as a varint
 *	with ZigZag encoding.
 *
 *	\tparam T
 *		The type of signed integer to serialize.
 *	\tparam Sink
 *		A type which models `Sink`.
 *
 *	\param [in] val
 *		The integer to serialize.
 *	\param [in] sink
 *		An object which models `Sink` to which
 *		binary data shall be written.
 */
template <typename T, typename Sink>
void serialize_varint_zigzag (T val, Sink & sink) {
	detail::zigzag<T>();
	auto u = detail::to_zigzag(val);
	protocol::serialize_varint(u, sink);
}

}
}
