/**
 *	\file
 */

#pragma once

#include <beast/core/error.hpp>
#include <mcpp/optional.hpp>
#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	Represents an error reported by the
 *	Yggdrasil API.
 *
 *	\sa
 *		error
 */
class api_error {
public:
	api_error () = default;
	api_error (const api_error &) = default;
	api_error (api_error &&) = default;
	api_error & operator = (const api_error &) = default;
	api_error & operator = (api_error &&) = default;
	/**
	 *	Creates a new api_error while filling
	 *	in all members thereof.
	 *
	 *	\param [in] error
	 *		A short description of the error.
	 *	\param [in] error_message
	 *		A longer more informative description
	 *		of the error.
	 *	\param [in] cause
	 *		An optional string which describes the
	 *		underlying cause of the error. Defaults
	 *		to an empty optional.
	 */
	api_error (
		std::string error,
		std::string error_message,
		optional<std::string> cause = nullopt
	);
	/**
	 *	A short description of the error.
	 */
	std::string error;
	/**
	 *	A long description of the error which
	 *	can be shown to a user.
	 */
	std::string error_message;
	/**
	 *	The cause of the error if available.
	 */
	optional<std::string> cause;
};

/**
 *	Represents an error resulting from an
 *	operation against the Yggdrasil API.
 *
 *	\sa
 *		api_error
 */
class error : public beast::error_code {
public:
	using beast::error_code::error_code;
	//	For some reason this is necessary even
	//	given the above line and the fact that
	//	beast::error_code is default constructible
	error () = default;
	/**
	 *	Creates a new error object.
	 *
	 *	\param [in] code
	 *		The error code.
	 *	\param [in] api
	 *		An optional \ref api_error object
	 *		representing the error reported by
	 *		the Yggdrasil API. If the newly-created
	 *		error object is not to represent an error
	 *		reported by the Yggdrasil API but instead
	 *		to represent another error which occurred
	 *		in the course of communicating with the
	 *		Yggdrasil API then this should be empty
	 *		Defaults to an empty optional.
	 */
	explicit error (
		beast::error_code code,
		optional<api_error> api = nullopt
	);
	/**
	 *	The error reported by Yggdrasil, if any.
	 *
	 *	If this field is empty it should be assumed
	 *	that the Yggdrasil API could not be contacted
	 *	or that it behaved in an unexpected manner.
	 */
	optional<api_error> api;
};

}
}
