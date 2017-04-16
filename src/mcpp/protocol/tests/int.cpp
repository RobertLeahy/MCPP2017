#include <mcpp/protocol/int.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/protocol/error.hpp>
#include <mcpp/protocol/exception.hpp>
#include <cstdint>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace tests {
namespace {

SCENARIO("Integers may be parsed", "[mcpp][protocol][int]") {
	GIVEN("A buffer containing a representation of an unsigned integer") {
		unsigned char buf [] = {0, 16};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_int<std::uint16_t>(b);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == 16);
				}
			}
		}
	}
	GIVEN("A buffer containing a representation of a signed integer") {
		unsigned char buf [] = {255, 255, 255, 253};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_int<std::int32_t>(b);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == -3);
				}
			}
		}
	}
	GIVEN("A buffer which is too short to contain an integer representation") {
		unsigned char buf [] = {0};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_int<std::int64_t>(b);
			THEN("The parse fails") {
				REQUIRE_FALSE(result);
				auto && e = result.error();
				AND_THEN("The correct error code is returned") {
					CHECK(e == make_error_code(error::end_of_file));
				}
			}
		}
	}
}

SCENARIO("Integers may be parsed and the result thereof may be assigned to some variable", "[mcpp][protocol][int]") {
	GIVEN("A buffer containing a representation of an unsigned integer") {
		unsigned char buf [] = {0, 32};
		buffer b(buf);
		WHEN("It is parsed") {
			std::uint16_t res;
			auto result = parse_int(b, res);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(res == 32);
				}
			}
		}
	}
}

SCENARIO("Functors which parse integers may be created", "[mcpp][protocol][int]") {
	GIVEN("A buffer containing the representation of two integers") {
		unsigned char buf [] = {0, 64, 0, 128};
		buffer b(buf);
		WHEN("A functor which parses from that buffer and assigns to an integer is created") {
			std::uint16_t res;
			auto func = make_int_parser(b, res);
			AND_WHEN("The functor is invoked") {
				auto result = func();
				THEN("The parse succeeds") {
					REQUIRE(result);
					AND_THEN("The first integer is parsed") {
						CHECK(res == 64);
					}
				}
				AND_WHEN("The functor is invoked again") {
					result = func();
					THEN("The parse succeds") {
						REQUIRE(result);
						AND_THEN("The second integer is parsed") {
							CHECK(res == 128);
						}
					}
				}
			}
		}
	}
}

SCENARIO("Integers may be serialized", "[mcpp][protocol][int]") {
	GIVEN("A sufficiently sized buffer") {
		unsigned char buf [4];
		buffer b(buf);
		WHEN("An unsigned integer is serialized") {
			std::uint8_t byte(1);
			serialize_int(byte, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 1);
				AND_THEN("The correct bytes are written") {
					CHECK(buf[0] == 1);
				}
			}
		}
		WHEN("A signed integer is serialized") {
			std::int32_t i = -2;
			serialize_int(i, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 4);
				AND_THEN("The correct bytes are written") {
					CHECK(buf[0] == 255);
					CHECK(buf[1] == 255);
					CHECK(buf[2] == 255);
					CHECK(buf[3] == 254);
				}
			}
		}
	}
	GIVEN("A buffer which is too small") {
		buffer b;
		WHEN("An integer is serialized") {
			THEN("An exception is thrown") {
				CHECK_THROWS_AS(serialize_int(1, b), write_overflow_error);
			}
		}
	}
}

}
}
}
}
