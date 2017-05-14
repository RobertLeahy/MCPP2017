/**
 *	\file
 */

#pragma once

#include <string>

namespace mcpp {
namespace yggdrasil {

/**
 *	Encapsulates the agent performing requests
 *	against Yggdrasil.
 */
class agent {
public:
	agent ();
	agent (const agent &) = default;
	agent (agent &&) = default;
	agent & operator = (const agent &) = default;
	agent & operator = (agent &&) = default;
	/**
	 *	Creates an agent object.
	 *
	 *	\param [in] name
	 *		The name of the agent.
	 *	\param [in] version
	 *		The version of the agent.
	 */
	agent (std::string name, unsigned version);
	/**
	 *	The name of the agent. For Minecraft
	 *	set to "Minecraft," for Scrolls set
	 *	to "Scrolls."
	 */
	std::string name;
	/**
	 *	The version of the agent. Usually 1.
	 */
	unsigned version;
};

}
}
