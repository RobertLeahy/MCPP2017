/**
 *	\file
 */

#pragma once

#include <string>
#include <system_error>
#include <type_traits>

namespace mcpp {
namespace protocol {

/**
 *	An enumeration of parse-related error
 *	codes.
 */
enum class error {
	end_of_file,	/**<	EOF was encountered unexpectedly while parsing	*/
	unrepresentable,	/**<	The encoded value does not fit into the type to be used to represent it	*/
	overlong,	/**<	A variable width encoding was wider than it needed to be	*/
	overflow,	/**<	An integer overflow occurred during parsing	*/
	unexpected	/**<	One of a set of values was expected but the given value was not among those	*/
};

/**
 *	Obtains a human readable message for an
 *	\ref error.
 *
 *	\param [in] c
 *		The \ref error value.
 *
 *	\return
 *		A string.
 */
const std::string & to_string (error c);

/**
 *	Returns a reference to a `std::error_category`
 *	object which allows \ref error enumeration values
 *	to serve in `std::error_code` and `std::error_condition`
 *	objects.
 */
const std::error_category & error_category ();

/**
 *	Creates a `std::error_code` object from an \ref error
 *	enumeration value.
 *
 *	\param [in] e
 *		A \ref error enumeration value.
 *
 *	\return
 *		A `std::error_code`.
 */
std::error_code make_error_code (error e) noexcept;

/**
 *	Creates a `std::error_condition` object from an
 *	\ref error enumeration value.
 *
 *	\param [in] e
 *		A \ref error enumeration value.
 *
 *	\return
 *		A `std::error_condition`.
 */
std::error_condition make_error_condition (error e) noexcept;

}
}

namespace std {

template <>
struct is_error_code_enum<mcpp::protocol::error> : public std::true_type {	};
template <>
struct is_error_condition_enum<mcpp::protocol::error> : public std::true_type {	};

}
