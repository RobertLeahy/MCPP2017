/**
 *	\file
 */

#pragma once

#include <string>
#include <unordered_map>

namespace mcpp {
namespace yggdrasil {

/**
 *	Represents a user and its properties.
 */
class user {
public:
	user () = default;
	user (const user &) = default;
	user (user &&) = default;
	user & operator = (const user &) = default;
	user & operator = (user &&) = default;
	/**
	 *	A map from strings to strings which represents
	 *	the properties of a user.
	 */
	using properties_type = std::unordered_map<std::string, std::string>;
	/**
	 *	Creates a user object.
	 *
	 *	\param [in] id
	 *		A hexadecimal string identifying the
	 *		user.
	 *	\param [in] properties
	 *		The properties of the user.
	 */
	user (std::string id, properties_type properties);
	/**
	 *	A hexadecimal string identifying the
	 *	user.
	 */
	std::string id;
	/**
	 *	The properties associated with the user.
	 */
	properties_type properties;
};

}
}
