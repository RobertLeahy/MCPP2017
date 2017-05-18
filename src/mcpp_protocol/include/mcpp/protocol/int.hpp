/**
 *	\file
 */

#pragma once

#include "error.hpp"
#include "exception.hpp"
//	In Boost 1.61.0 including boost/endian/endian.hpp
//	is an error whereas in Boost 1.55.0 boost/endian/conversion.hpp
//	doesn't exist apparently
#ifdef MCPP_HAS_BOOST_ENDIAN_CONVERSION
#include <boost/endian/conversion.hpp>
#else
#include <boost/endian/endian.hpp>
#endif
#include <boost/expected/expected.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
#include <mcpp/iostreams/traits.hpp>
#include <cstddef>
#include <cstring>
#include <ios>
#include <system_error>

namespace mcpp {
namespace protocol {

/**
 *	Parses an integer from a binary stream.
 *
 *	\tparam T
 *		The type of integer to parse.
 *	\tparam Source
 *		A type which models `Source`.
 *
 *	\param [in] src
 *		An object which models `Source` from which to
 *		read raw bytes.
 *
 *	\return
 *		The integer which was parsed. If an integer
 *		could not be parsed a `std::error_code` object
 *		will be returned encapsulating the reason for the
 *		failure.
 */
template <typename T, typename Source>
boost::expected<T, std::error_code> parse_int (Source & src) {
	constexpr std::size_t size = sizeof(T);
	iostreams::char_type_of_t<Source> buffer [size];
	auto i = boost::iostreams::read(src, buffer, size);
	if ((i == -1) || (std::size_t(i) != size)) return boost::make_unexpected(
		make_error_code(error::end_of_file)
	);
	T retr;
	std::memcpy(&retr, buffer, size);
	boost::endian::big_to_native_inplace(retr);
	return retr;
}
/**
 *	Functions identically to \ref parse_int except assigns
 *	the parsed integer to a variable rather than transmitting
 *	it through its return value.
 *
 *	This allows consumers to avoid specifying \em T as a template
 *	parameter (as it can be deduced) and also abstracts more error
 *	handling away (as the assignment is not performed on error).
 *
 *	\tparam Source
 *		A type which models `Source`.
 *	\tparam T
 *		The type of integer to parse.
 *
 *	\param [in] src
 *		An object which models `Source` from which to read raw
 *		bytes.
 *	\param [out] val
 *		The variable to assign the result to. On error the value
 *		of this variable is unspecified.
 *
 *	\return
 *		Nothing on success. A `std::error_code` object if the
 *		parse failed.
 */
template <typename Source, typename T>
boost::expected<void, std::error_code> parse_int (Source & src, T & val) {
	return protocol::parse_int<T>(src).map([&] (auto i) noexcept {	val = i;	});
}

/**
 *	Creates a function which parses an integer from `Source`,
 *	assigns the result thereof (if applicable) to a provided
 *	integer, and which returns the overall result as a
 *	`boost::expected<void, std::error_code>`.
 *
 *	This function makes it more convenient to chain parse
 *	operations together with `boost::expected::bind` and
 *	`boost::expected::map`.
 *
 *	\tparam Source
 *		A type which models `Source`.
 *	\tparam T
 *		The type of integer to parse and assign.
 *
 *	\param [in] src
 *		The `Source` from which an integer shall be parsed
 *		whenever the resulting function is invoked. This
 *		reference must remain valid as long as the resulting
 *		function may be invoked or the behavior is undefined.
 *	\param [in] val
 *		The integer to which the result of each parse operation
 *		shall be assigned. If a parse operation fails the value
 *		of this integer is unspecified. This reference must remain
 *		valid as long as the resulting function may be invoked
 *		or the behavior is undefined.
 *
 *	\return
 *		A function as described above.
 */
template <typename Source, typename T>
auto make_int_parser (Source & src, T & val) noexcept {
	return [&] () {	return protocol::parse_int(src, val);	};
}

/**
 *	Serializes an integer to a binary stream.
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
 *		bytes shall be written.
 */
template <typename T, typename Sink>
void serialize_int (T val, Sink & sink) {
	boost::endian::native_to_big_inplace(val);
	//	TODO: Fix this for types that don't
	//	have strict aliasing exemption?
	auto ptr = reinterpret_cast<const iostreams::char_type_of_t<Sink> *>(&val);
	constexpr std::size_t size = sizeof(T);
	std::size_t i(boost::iostreams::write(sink, ptr, std::streamsize(size)));
	if (i != size) throw write_overflow_error(size, i);
}

}
}
