/**
 *	\file
 */

#pragma once

#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	Invalidates an access token using a username
 *	and password.
 */
class signout_request {
public:
	signout_request () = default;
	signout_request (const signout_request &) = default;
	signout_request (signout_request &&) = default;
	signout_request & operator = (const signout_request &) = default;
	signout_request & operator = (signout_request &&) = default;
	/**
	 *	Creates a new signout_request.
	 *
	 *	\param [in] username
	 *		The username.
	 *	\param [in] password
	 *		The password.
	 */
	signout_request (std::string username, std::string password);
	/**
	 *	The username.
	 */
	std::string username;
	/**
	 *	The password.
	 */
	std::string password;
};

/**
 *	The Yggdrasil API does not return a response
 *	to a signout request. It simply succeeds or
 *	fails. Accordingly this is simply a type alias
 *	for `void`.
 *
 *	\sa
 *		signout_request
 */
using signout_response = void;

}
}
