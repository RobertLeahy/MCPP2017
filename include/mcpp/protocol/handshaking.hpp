/**
 *	\file
 */

#pragma once

#include "direction.hpp"
#include "error.hpp"
#include "exception.hpp"
#include "int.hpp"
#include "packet.hpp"
#include "packet_parameters.hpp"
#include "parameterized_packet_serializer.hpp"
#include "state.hpp"
#include "string.hpp"
#include "varint.hpp"
#include <boost/expected/expected.hpp>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace mcpp {
namespace protocol {
namespace handshaking {

namespace serverbound {

/**
 *	Causes the server to switch into the target state.
 *
 *	\tparam PacketParameters
 *		A model of `PacketParameters` which shall be used
 *		to configure the packet.
 */
template <typename PacketParameters>
class basic_handshake : public packet {
public:
	std::uint32_t protocol_version;
	/**
	 *	Host name or IP that was used to connect.
	 *	Vanilla server ignores this information.
	 */
	string_t<PacketParameters> server_address;
	/**
	 *	Port that was used to connect. Vanilla
	 *	server ignores this information.
	 */
	std::uint16_t server_port;
	/**
	 *	Either \ref state::status or \ref state::login,
	 *	the connection shall switch to the designated
	 *	state.
	 */
	state next_state;
	explicit basic_handshake (const allocator_t<PacketParameters> & a = allocator_t<PacketParameters>{})
		:	server_address(create_string<PacketParameters>(a))
	{	}
};

/**
 *	A convenience type alias for \ref basic_handshake
 *	which uses \ref packet_parameters for the
 *	\em PacketParameters template parameter.
 */
using handshake = basic_handshake<packet_parameters>;

template <typename Source, typename Sink, typename PacketParameters = packet_parameters>
class handshake_serializer final : public parameterized_packet_serializer<
	basic_handshake,
	0,
	direction::serverbound,
	state::handshaking,
	Source,
	Sink,
	PacketParameters
> {
private:
	using base = parameterized_packet_serializer<
		basic_handshake,
		0,
		direction::serverbound,
		state::handshaking,
		Source,
		Sink,
		PacketParameters
	>;
public:
	using base::base;
	virtual void serialize (const typename base::packet_type & p, Sink & sink) const override {
		serialize_varint(p.protocol_version, sink);
		serialize_string(p.server_address, sink);
		serialize_int(p.server_port, sink);
		unsigned char s;
		switch (p.next_state) {
		case state::status:
			s = 1;
			break;
		case state::login:
			s = 2;
			break;
		default:{
			std::ostringstream ss;
			ss << "Unable to represent " << to_string(p.next_state) << " in handshaking::serverbound::handshake";
			throw unrepresentable_error(ss.str());
		}break;
		}
		serialize_int(s, sink);
	}
	virtual typename base::parse_result_type parse (Source & src, typename base::pointer & ptr) const override {
		auto && p = ptr.template emplace<typename base::packet_type>(base::get_allocator());
		using result_type = typename base::parse_result_type;
		return protocol::parse_varint(src, p.protocol_version).bind(
			protocol::make_string_parser(src, p.server_address)
		).bind(
			protocol::make_int_parser(src, p.server_port)
		).bind([&] () {
			return protocol::parse_int<unsigned char>(src).bind([&] (auto s) noexcept -> result_type {
				switch (s) {
				case 1:
					p.next_state = state::status;
					return result_type{};
				case 2:
					p.next_state = state::login;
					return result_type{};
				default:
					return boost::make_unexpected(make_error_code(error::unexpected));
				}
				return result_type{};
			});
		});
	}
};

}

}
}
}
