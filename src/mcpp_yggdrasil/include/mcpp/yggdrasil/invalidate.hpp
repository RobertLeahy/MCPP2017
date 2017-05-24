/**
 *	\file
 */

#pragma once

#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	Represents a request to invalidate an
 *	access token using an access token and
 *	client token.
 *
 *	\sa
 *		invalidate_response
 */
class invalidate_request {
public:
	invalidate_request () = default;
	invalidate_request (const invalidate_request &) = default;
	invalidate_request (invalidate_request &&) = default;
	invalidate_request & operator = (const invalidate_request &) = default;
	invalidate_request & operator = (invalidate_request &&) = default;
	/**
	 *	Creates a new invalidate_request.
	 *
	 *	\param [in] access_token
	 *		The access token.
	 *	\param [in] client_token
	 *		The client token.
	 */
	invalidate_request (std::string access_token, std::string client_token);
	/**
	 *	The access token to invalidate.
	 */
	std::string access_token;
	/**
	 *	The client token. Needs to be identical
	 *	to the token used to obtain the access
	 *	token.
	 */
	std::string client_token;
};

/**
 *	Requests to invalidate an access token
 *	do not return a payload. Accordingly this
 *	is simply a type alias for `void`.
 *
 *	\sa
 *		invalidate_request
 */
using invalidate_response = void;

}
}
