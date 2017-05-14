/**
 *	\file
 */

#pragma once

namespace mcpp {
namespace protocol {

/**
 *	A base class for Minecraft protocol packets.
 *
 *	Provides no methods only a virtual destructor
 *	so that the type of the derived packet may be
 *	detected via RTTI.
 */
class packet {
public:
	packet () = default;
	packet (const packet &) = default;
	packet (packet &&) = default;
	packet & operator = (const packet &) = default;
	packet & operator = (packet &&) = default;
	/**
	 *	Provided so that derived class
	 *	types may be detected via RTTI.
	 */
	virtual ~packet () noexcept;
};

}
}
