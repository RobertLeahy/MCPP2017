#include <mcpp/protocol/handshaking.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/protocol/packet_parameters.hpp>
#include <mcpp/protocol/state.hpp>
#include <algorithm>
#include <iterator>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace handshaking {
namespace tests {
namespace {

SCENARIO("handshaking::serverbound::handshake packets may be parsed", "[mcpp][protocol][handshaking][serverbound][handshake]") {
	GIVEN("A handshake_serializer") {
		serverbound::handshake_serializer<buffer, buffer, packet_parameters> ser;
		decltype(ser)::pointer ptr;
		WHEN("A buffer containing the representation of a handshaking::serverbound::handshake packet is parsed") {
			unsigned char buf [] = {
				0b10111100, 0b00000010,
				4, 't', 'e', 's', 't',
				0b01100011, 0b11011101,
				1
			};
			buffer b(buf);
			auto result = ser.parse(b, ptr);
			THEN("A packet is parsed") {
				REQUIRE(result);
				REQUIRE(ptr);
				AND_THEN("The contents of that packet are correct") {
					auto & packet = dynamic_cast<serverbound::handshake &>(*ptr);
					CHECK(packet.protocol_version == 316);
					CHECK(packet.server_address == "test");
					CHECK(packet.server_port == 25565);
					CHECK(packet.next_state == state::status);
				}
			}
		}
	}
}

SCENARIO("handshaking::serverbound::handshake packets may be serialized", "[mcpp][protocol][handshaking][serverbound][handshake]") {
	GIVEN("A handshake_serializer") {
		serverbound::handshake_serializer<buffer, buffer, packet_parameters> ser;
		decltype(ser)::pointer ptr;
		WHEN("A handshaking::serverbound::handshake packet is serialized") {
			serverbound::handshake packet;
			packet.protocol_version = 316;
			packet.server_address = "test";
			packet.server_port = 25565;
			packet.next_state = state::status;
			unsigned char buf [10];
			buffer b(buf);
			ser.serialize(packet, b);
			THEN("The correct representation thereof is written") {
				unsigned char expected [] = {
					0b10111100, 0b00000010,
					4, 't', 'e', 's', 't',
					0b01100011, 0b11011101,
					1
				};
				using std::begin;
				using std::end;
				CHECK(std::equal(begin(expected), end(expected), begin(buf), begin(buf) + b.written()));
			}
		}
	}
}

}
}
}
}
}
