#include <mcpp/protocol/incremental_varint_parser.hpp>
#include <mcpp/buffer.hpp>
#include <cstddef>
#include <cstdint>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace tests {
namespace {

SCENARIO("mcpp::protocol::incremental_varint_parser may be used to parse a Varint in multiple passes", "[mcpp][protocol][incremental_varint_parser]") {
	GIVEN("An mcpp::protocol::incremental_varint_parser") {
		incremental_varint_parser<std::uint16_t> parser;
		WHEN("An entire varint is parsed at once") {
			unsigned char buf [] = {1};
			buffer b(buf);
			auto result = parser.parse(b);
			THEN("The parse is successful") {
				REQUIRE(result);
				AND_THEN("A value is parsed") {
					REQUIRE(*result);
					AND_THEN("The parsed value is correct") {
						CHECK(**result == 1);
					}
				}
			}
			AND_WHEN("Another varint is parsed at once") {
				unsigned char buf [] = {2};
				buffer b(buf);
				auto result = parser.parse(b);
				THEN("The same value is returned as mcpp::protocol::incremental_varint_parser::reset was not invoked") {
					REQUIRE(result);
					REQUIRE(*result);
					CHECK(**result == 1);
				}
			}
			AND_WHEN("mcpp::protocol::incremental_varint_parser::reset is invoked") {
				parser.reset();
				AND_WHEN("Another varint is parsed at once") {
					unsigned char buf [] = {2};
					buffer b(buf);
					auto result = parser.parse(b);
					THEN("The parse is successful") {
						REQUIRE(result);
						AND_THEN("A value is parsed") {
							REQUIRE(*result);
							AND_THEN("The parsed value is correct") {
								CHECK(**result == 2);
							}
						}
					}
				}
			}
		}
		WHEN("Part of a varint is parsed") {
			unsigned char buf [] = {128};
			buffer b(buf);
			auto result = parser.parse(b);
			THEN("The parse is successful") {
				REQUIRE(result);
				AND_THEN("No value is parsed") {
					CHECK_FALSE(*result);
				}
			}
			AND_WHEN("The remainder thereof is parsed") {
				unsigned char buf [] = {1};
				buffer b(buf);
				auto result = parser.parse(b);
				THEN("The parse is successful") {
					REQUIRE(result);
					AND_THEN("A value is parsed") {
						REQUIRE(*result);
						AND_THEN("The parsed value is correct") {
							CHECK(**result == 128);
						}
					}
				}
			}
		}
	}
}

}
}
}
}
