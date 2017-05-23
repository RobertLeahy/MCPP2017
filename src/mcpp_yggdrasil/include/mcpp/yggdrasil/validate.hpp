/**
 *	\file
 */

#pragma once

#include <mcpp/optional.hpp>
#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	A request to check Yggdrasil to see if an
 *	access token is usable to authenticate with
 *	a Minecraft server.
 *
 *	Note that an access token may not be usable
 *	to authenticate with a Minecraft server, but
 *	it may still be possible to refresh that
 *	access token (see \ref refresh_request).
 *
 *	\sa
 *		validate_response
 */
class validate_request {
public:
	validate_request () = default;
	validate_request (const validate_request &) = default;
	validate_request (validate_request &&) = default;
	validate_request & operator = (const validate_request &) = default;
	validate_request & operator = (validate_request &&) = default;
	/**
	 *	Creates a new validate_request.
	 *
	 *	\param [in] access_token
	 *		The access token to validate.
	 *	\param [in] client_token
	 *		The client token, if applicable. Defaults
	 *		to an empty optional.
	 */
	explicit validate_request (
		std::string access_token,
		optional<std::string> client_token = nullopt
	);
	/**
	 *	The access token to validate.
	 */
	std::string access_token;
	/**
	 *	The client token, if applicable.
	 *
	 *	A validate request may be made with or
	 *	without a client token. If one is provided
	 *	it should be the same as the client token
	 *	which was used to obtain the corresponding
	 *	access token.
	 */
	optional<std::string> client_token;
};

/**
 *	The response to a validate request is
 *	either that the client token/access token
 *	pair is valid or that it is not. \em true
 *	is returned in the former case, \em false
 *	in the matter. Accordingly this is simply
 *	a type alias for `bool`.
 *
 *	\sa
 *		validate_request
 */
using validate_response = bool;

}
}
