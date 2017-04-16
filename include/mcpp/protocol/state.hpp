/**
 *	\file
 */

#pragma once

#include "../enum_hash.hpp"
#include <functional>
#include <string>

namespace mcpp {
namespace protocol {

/**
 *	Enumerates the various states a Minecraft
 *	client connection may be in.
 */
enum class state {
	handshaking,	/**<	Deciding whether to transition to \ref status or \ref login	*/
	play,	/**<	In game	*/
	status,	/**<	Querying server status	*/
	login	/**<	Authenticating	*/
};

/**
 *	Obtains a human readable name for a \ref state
 *	enumeration value.
 *
 *	\param [in] s
 *		The \ref state value.
 *
 *	\return
 *		A string.
 */
const std::string & to_string (state s);

}
}

namespace std {

template <>
struct hash<mcpp::protocol::state> : public mcpp::enum_hash<mcpp::protocol::state> {	};

}
