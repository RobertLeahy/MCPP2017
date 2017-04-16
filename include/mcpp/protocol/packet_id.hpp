/**
 *	\file
 */

#pragma once

#include "direction.hpp"
#include "state.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>

namespace mcpp {
namespace protocol {

/**
 *	Identifies a packet as a triple of
 *	\ref state, \ref direction, and numeric
 *	ID.
 */
class packet_id {
public:
	packet_id () = delete;
	packet_id (const packet_id &) = default;
	packet_id (packet_id &&) = default;
	packet_id & operator = (const packet_id &) = default;
	packet_id & operator = (packet_id &&) = default;
	/**
	 *	Creates a packet_id.
	 *
	 *	\param [in] id
	 *		The numeric ID.
	 *	\param [in] d
	 *		The \ref protocol::direction in which the
	 *		packet is sent.
	 *	\param [in] s
	 *		The \ref protocol::state in which the packet
	 *		is sent and received.
	 */
	packet_id (std::uint32_t id, protocol::direction d, protocol::state s) noexcept;
	/**
	 *	Retrieves the numeric ID which identifies the
	 *	packet on the wire.
	 *
	 *	\return
	 *		The ID.
	 */
	std::uint32_t id () const noexcept;
	/**
	 *	Retrieves the \ref protocol::direction representing
	 *	the direction in which the identified packet
	 *	is sent.
	 *
	 *	\return
	 *		The direction.
	 */
	protocol::direction direction () const noexcept;
	/**
	 *	Retrieves the \ref protocol::state representing
	 *	the connection state in which the identified
	 *	packet is sent and received.
	 *
	 *	\return
	 *		The state.
	 */
	protocol::state state () const noexcept;
private:
	std::uint32_t id_;
	protocol::direction d_;
	protocol::state s_;
};

bool operator == (const packet_id &, const packet_id &) noexcept;
bool operator != (const packet_id &, const packet_id &) noexcept;
bool operator < (const packet_id &, const packet_id &) noexcept;

}
}

namespace std {

template <>
struct hash<mcpp::protocol::packet_id> {
public:
	using result_type = std::size_t;
	using argument_type = const mcpp::protocol::packet_id &;
	result_type operator () (argument_type) const noexcept;
};

}
