/**
 *	\file
 */

#pragma once

#include <string>

namespace mcpp {
namespace protocol {

/**
 *	An enumeration of parse-related error
 *	codes.
 */
enum class error_code {
	end_of_file,	/**<	EOF was encountered unexpectedly while parsing	*/
	unrepresentable,	/**<	The encoded value does not fit into the type to be used to represent it	*/
	overlong	/**<	A variable width encoding was wider than it needed to be	*/
};

/**
 *	Obtains a human readable message for an
 *	\ref error_code.
 *
 *	\param [in] c
 *		The \ref error_code value.
 *
 *	\return
 *		A string.
 */
const std::string & to_string (error_code c);

}
}
