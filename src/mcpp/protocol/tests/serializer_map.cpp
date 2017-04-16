#include <mcpp/protocol/serializer_map.hpp>
#include <mcpp/protocol/direction.hpp>
#include <mcpp/protocol/handshaking.hpp>
#include <mcpp/protocol/packet_id.hpp>
#include <mcpp/protocol/state.hpp>
#include <streambuf>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace tests {
namespace {

SCENARIO("mcpp::protocol::serializer_map may be used to obtain a default serializer map", "[mcpp][protocol][serializer_map]") {
	GIVEN("The result of calling mcpp::protocol::serializer_map") {
		auto map = serializer_map<std::streambuf, std::streambuf>();
		using serializer_type = handshaking::serverbound::handshake_serializer<std::streambuf, std::streambuf>;
		WHEN("A serializer is looked up by packet ID") {
			packet_id id(0, direction::serverbound, state::handshaking);
			auto ptr = get(map, id);
			THEN("A serializer is found") {
				REQUIRE(ptr);
				AND_THEN("It is the correct serializer") {
					CHECK(dynamic_cast<const serializer_type *>(ptr));
				}
			}
		}
		WHEN("A serializer is looked up by packet") {
			handshaking::serverbound::handshake packet;
			auto ptr = get(map, packet);
			THEN("A serializer is found") {
				REQUIRE(ptr);
				AND_THEN("It is the correct serializer") {
					CHECK(dynamic_cast<const serializer_type *>(ptr));
				}
			}
		}
		WHEN("A serializer is looked up by std::type_info") {
			auto ptr = get(map, typeid(handshaking::serverbound::handshake));
			THEN("A serializer is found") {
				REQUIRE(ptr);
				AND_THEN("It is the correct serializer") {
					CHECK(dynamic_cast<const serializer_type *>(ptr));
				}
			}
		}
	}
}

}
}
}
}
