/**
 *	\file
 */

#pragma once

#include "agent.hpp"
#include "profile.hpp"
#include "user.hpp"
#include <mcpp/optional.hpp>
#include <string>
#include <vector>

namespace mcpp {
namespace yggdrasil {

/**
 *	Represents a request to log a user in.
 *
 *	\sa
 *		authenticate_response
 */
class authenticate_request {
public:
	authenticate_request ();
	authenticate_request (const authenticate_request &) = default;
	authenticate_request (authenticate_request &&) = default;
	authenticate_request & operator = (const authenticate_request &) = default;
	authenticate_request & operator = (authenticate_request &&) = default;
	/**
	 *	Creates an authenticate_request.
	 *
	 *	\param [in] username
	 *		A string giving the username to use to
	 *		login.
	 *	\param [in] password
	 *		A string giving the password to use to
	 *		login.
	 *	\param [in] agent
	 *		The \ref yggdrasil::agent submitting the
	 *		request. Defaults to empty.
	 *	\param [in] client_token
	 *		The client token. Defaults to empty.
	 *	\param [in] request_user
	 *		\em true to request a \ref user object
	 *		in the response. Defaults to \em false.
	 */
	authenticate_request (
		std::string username,
		std::string password,
		optional<yggdrasil::agent> agent = optional<yggdrasil::agent>{},
		optional<std::string> client_token = optional<std::string>{},
		bool request_user = false
	);
	/**
	 *	The \ref agent submitting the request.
	 */
	optional<yggdrasil::agent> agent;
	/**
	 *	The username to use to login.
	 */
	std::string username;
	/**
	 *	The password to use to login.
	 */
	std::string password;
	/**
	 *	If present a randomly generated identifier
	 *	which must be the same for each request.
	 */
	optional<std::string> client_token;
	/**
	 *	If \em true adds a \ref user object to the
	 *	response.
	 */
	bool request_user;
};

/**
 *	Represents a response to a request to log a user
 *	in.
 *
 *	\sa
 *		authenticate_request
 */
class authenticate_response {
public:
	authenticate_response () = default;
	authenticate_response (const authenticate_response &) = default;
	authenticate_response (authenticate_response &&) = default;
	authenticate_response & operator = (const authenticate_response &) = default;
	authenticate_response & operator = (authenticate_response &&) = default;
	/**
	 *	A vector of \ref profile objects.
	 */
	using available_profiles_type = std::vector<profile>;
	/**
	 *	Creates a new authenticate_response.
	 *
	 *	\param [in] access_token
	 *		The access token. Usually hexadecimal.
	 *	\param [in] client_token
	 *		The client token that was sent in the
	 *		\ref authenticate_request.
	 *	\param [in] available_profiles
	 *		A list of the available profiles unless
	 *		no \ref agent was sent in the \ref authenticate_request.
	 *		Defaults to empty.
	 *	\param [in] selected_profile
	 *		The selected \ref profile unless no
	 *		\ref agent was sent in the
	 *		\ref authenticate_request. Defaults to
	 *		empty.
	 *	\param [in] user
	 *		The \ref user object if
	 *		\ref authenticate_request::request_user
	 *		was set to \em true in the request.
	 */
	authenticate_response (
		std::string access_token,
		std::string client_token,
		optional<available_profiles_type> available_profiles = optional<available_profiles_type>{},
		optional<profile> selected_profile = optional<profile>{},
		optional<yggdrasil::user> user = optional<yggdrasil::user>{}
	);
	/**
	 *	Usually hexadecimal.
	 */
	std::string access_token;
	/**
	 *	Identical to the one received in the
	 *	\ref authenticate_request.
	 */
	std::string client_token;
	/**
	 *	Only present if \ref authenticate_request::agent was
	 *	received.
	 */
	optional<available_profiles_type> available_profiles;
	/**
	 *	Only present if \ref authenticate_request::agent was
	 *	received.
	 */
	optional<profile> selected_profile;
	/**
	 *	Only present if \ref authenticate_request::request_user
	 *	was \em true.
	 */
	optional<yggdrasil::user> user;
};

}
}
