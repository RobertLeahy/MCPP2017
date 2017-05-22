/**
 *	\file
 */

#pragma once

#include "profile.hpp"
#include "user.hpp"
#include <mcpp/optional.hpp>
#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	Represents a request to refresh a valid
 *	access token.
 *
 *	\sa
 *		refresh_response
 */
class refresh_request {
public:
	refresh_request ();
	refresh_request (const refresh_request &) = default;
	refresh_request (refresh_request &&) = default;
	refresh_request & operator = (const refresh_request &) = default;
	refresh_request & operator = (refresh_request &&) = default;
	/**
	 *	Creates a refresh_request.
	 *
	 *	\param [in] access_token
	 *		The access token to refresh.
	 *	\param [in] client_token
	 *		The client token associated with the
	 *		given access token.
	 *	\param [in] selected_profile
	 *		The profile to select. Defaults to an
	 *		empty optional.
	 *	\param [in] request_user
	 *		If \em true a \ref user object shall be
	 *		added to the response. Defaults to
	 *		\em false.
	 */
	refresh_request (
		std::string access_token,
		std::string client_token,
		optional<profile> selected_profile = nullopt,
		bool request_user = false
	);
	/**
	 *	The access token to refresh.
	 */
	std::string access_token;
	/**
	 *	The client token to refresh. Must be
	 *	identical to the client token used to
	 *	first obtain the access token.
	 */
	std::string client_token;
	/**
	 *	Setting this will cause the Mojang Yggdrasil
	 *	API to return an error unconditionally.
	 */
	optional<profile> selected_profile;
	/**
	 *	If \em true adds a \ref user object to
	 *	the response.
	 */
	bool request_user;
};

/**
 *	Represents a response to a request to refresh
 *	a valid access token.
 *
 *	\sa
 *		refresh_request
 */
class refresh_response {
public:
	refresh_response () = default;
	refresh_response (const refresh_response &) = default;
	refresh_response (refresh_response &&) = default;
	refresh_response & operator = (const refresh_response &) = default;
	refresh_response & operator = (refresh_response &&) = default;
	/**
	 *	Creates a new refresh_response.
	 *
	 *	\param [in] access_token
	 *		The access token.
	 *	\param [in] client_token
	 *		The client token.
	 *	\param [in] selected_profile
	 *		The selected profile, if any. Defaults to
	 *		an empty optional.
	 *	\param [in] user
	 *		The \ref user object which represents
	 *		the user if \ref refresh_request::request_user
	 *		was \em true. Defaults to an empty optional.
	 */
	refresh_response (
		std::string access_token,
		std::string client_token,
		optional<profile> selected_profile = nullopt,
		optional<yggdrasil::user> user = nullopt
	);
	/**
	 *	The refreshed access token.
	 */
	std::string access_token;
	/**
	 *	Identical to the client token received.
	 */
	std::string client_token;
	/**
	 *	The selected profile.
	 */
	optional<profile> selected_profile;
	/**
	 *	A \ref user object representing the user
	 *	if \ref refresh_request::request_user
	 *	was \em true.
	 */
	optional<yggdrasil::user> user;
};

}
}
