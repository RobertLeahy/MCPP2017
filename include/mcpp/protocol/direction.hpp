/**
 *	\file
 */

#pragma once

#include "../enum_hash.hpp"
#include <functional>

namespace mcpp {
namespace protocol {

/**
 *	The directions in which a packet travels
 *	in the Minecraft protocol.
 */
enum class direction {
	clientbound,	/**<	From the server to the client	*/
	serverbound	/**<	From the client to the server	*/
};

/**
 *	Obtains a human readable name for a \ref direction
 *	enumeration value.
 *
 *	\param [in] d
 *		The \ref direction value.
 *
 *	\return
 *		A string.
 */
const std::string & to_string (direction d);

}
}

namespace std {

template <>
struct hash<mcpp::protocol::direction> : public mcpp::enum_hash<mcpp::protocol::direction> {	};

}
