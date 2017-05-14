#include <mcpp/protocol/varint.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/protocol/error.hpp>
#include <mcpp/protocol/exception.hpp>
#include <cstdint>
#include <limits>
#include <string>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace tests {
namespace {

static_assert(detail::number_of_bits<std::uint16_t> == 16, "Incorrect number of bits for 16 bit unsigned integer");
static_assert(detail::number_of_bits<std::int16_t> == 16, "Incorrect number of bits for 16 bit signed integer");
static_assert(detail::number_of_bits<std::uint32_t> == 32, "Incorrect number of bits for 32 bit unsigned integer");
static_assert(detail::number_of_bits<std::int32_t> == 32, "Incorrect number of bits for 32 bit signed integer");
static_assert(detail::number_of_bits<std::uint64_t> == 64, "Incorrect number of bits for 64 bit unsigned integer");
static_assert(detail::number_of_bits<std::int64_t> == 64, "Incorrect number of bits for 64 bit signed integer");

static_assert(detail::varint_whole_bytes<std::uint16_t> == 2, "Incorrect number of whole bytes for 16 bit unsigned integer");
static_assert(detail::varint_whole_bytes<std::int16_t> == 2, "Incorrect number of whole bytes for 16 bit signed integer");
static_assert(detail::varint_whole_bytes<std::uint32_t> == 4, "Incorrect number of whole bytes for 32 bit unsigned integer");
static_assert(detail::varint_whole_bytes<std::int32_t> == 4, "Incorrect number of whole bytes for 32 bit signed integer");
static_assert(detail::varint_whole_bytes<std::uint64_t> == 9, "Incorrect number of whole bytes for 64 bit unsigned integer");
static_assert(detail::varint_whole_bytes<std::int64_t> == 9, "Incorrect number of whole bytes for 64 bit signed integer");

using no_incomplete_bytes_t = char [56 / 8];

static_assert(detail::varint_remaining_bits<std::uint16_t> == 2, "Incorrect number of bits in last byte for 16 bit unsigned integer");
static_assert(detail::varint_remaining_bits<no_incomplete_bytes_t> == 0, "Remaining bits when type has number of bits evenly divisible by 7");

static_assert(detail::varint_bits_in_last_byte<std::uint16_t> == 2, "Incorrect number of bits in last byte for 16 bit unsigned integer");
static_assert(detail::varint_bits_in_last_byte<no_incomplete_bytes_t> == 7, "Incorrect number of bits in last byte for type with number of bits evenly divisible by 7");

static_assert(detail::varint_partial_byte<std::uint16_t> == 1, "No partial byte for 16 bit unsigned integer");
static_assert(detail::varint_partial_byte<no_incomplete_bytes_t> == 0, "Partial byte for type with size evenly divisible by 7");

static_assert(detail::varint_bits_in_overflow_mask<std::uint16_t> == 5, "Wrong number of bits in overflow mask for 16 bit unsigned integer");
static_assert(detail::varint_bits_in_overflow_mask<no_incomplete_bytes_t> == 0, "Bits in overflow mask for type with number of bits evenly divisible by 7");

static_assert(detail::varint_overflow_mask<std::uint16_t> == 0b1111100, "Wrong overflow mask for 16 bit unsigned integer");

static_assert(varint_size<std::uint16_t> == 3, "Incorrect size for 16 bit unsigned varint");
static_assert(varint_size<std::int16_t> == 3, "Incorrect size for 16 bit signed varint");
static_assert(varint_size<std::uint32_t> == 5, "Incorrect size for 32 bit unsigned varint");
static_assert(varint_size<std::int32_t> == 5, "Incorrect size for 32 bit signed varint");
static_assert(varint_size<std::uint64_t> == 10, "Incorrect size for 64 bit unsigned varint");
static_assert(varint_size<std::int64_t> == 10, "Incorrect size for 64 bit signed varint");
static_assert(varint_size<no_incomplete_bytes_t> == (sizeof(no_incomplete_bytes_t) * 8 / 7), "Incorrect size for type with size evenly divisible by 7");

SCENARIO("Unsigned varints may be parsed", "[mcpp][protocol][varint]") {
	GIVEN("The single byte representation of a varint") {
		unsigned char buf [] = {1};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint<unsigned>(b);
			THEN("An integer is parsed successfully") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == 1);
				}
			}
		}
	}
	GIVEN("A multi-byte representation of a varint") {
		unsigned char buf [] = {0b10101100, 0b00000010};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint<unsigned>(b);
			THEN("An integer is parsed successfully") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == 300);
				}
			}
		}
	}
	GIVEN("An incomplete varint representation") {
		unsigned char buf [] = {128};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint<unsigned>(b);
			THEN("A std::error_code object is returned") {
				REQUIRE_FALSE(result);
				auto && e = result.error();
				AND_THEN("The correct error code is returned") {
					CHECK(e == make_error_code(error::end_of_file));
				}
				AND_THEN("The correct error message is returned") {
					CHECK(std::string("Unexpected EOF") == e.message());
				}
			}
		}
	}
	GIVEN("A representation of a varint which does not fit into a 16 bit integer") {
		//	This varint has 21 bits
		unsigned char buf [] = {255, 255, 127};
		buffer b(buf);
		WHEN("It is parsed into a 16 bit integer") {
			auto result = parse_varint<std::uint16_t>(b);
			THEN("A std::error_code object is returned") {
				REQUIRE_FALSE(result);
				auto && e = result.error();
				AND_THEN("The correct error code is returned") {
					CHECK(e == make_error_code(error::unrepresentable));
				}
				AND_THEN("The correct error message is returned") {
					CHECK(std::string("Encoded value unrepresentable by destination type") == e.message());
				}
			}
		}
	}
	GIVEN("A representation of a varint which contains too many bytes for the destination type") {
		unsigned char buf [] = {255, 255, 129, 129};
		buffer b(buf);
		WHEN("It is parsed into a 16 bit integer") {
			auto result = parse_varint<std::uint16_t>(b);
			THEN("A std::error_code object is returned") {
				REQUIRE_FALSE(result);
				auto && e = result.error();
				AND_THEN("The correct error code is returned") {
					CHECK(e == make_error_code(error::unrepresentable));
				}
				AND_THEN("The correct error message is returned") {
					CHECK(std::string("Encoded value unrepresentable by destination type") == e.message());
				}
			}
		}
	}
	GIVEN("An overlong representation of a varint") {
		unsigned char buf [] = {255, 0};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint<unsigned>(b);
			THEN("A std::error_code object is returned") {
				REQUIRE_FALSE(result);
				auto && e = result.error();
				AND_THEN("The correct error code is returned") {
					CHECK(e == make_error_code(error::overlong));
				}
				AND_THEN("The correct error message is returned") {
					CHECK(std::string("Encoded representation longer than necessary") == e.message());
				}
			}
		}
	}
}

SCENARIO("Signed varints may be parsed", "[mcpp][protocol][varint]") {
	GIVEN("The representation of a positive varint") {
		unsigned char buf [] = {1};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint<int>(b);
			THEN("It is parsed successfully") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == 1);
				}
			}
		}
	}
	GIVEN("The representation of a negative varint") {
		unsigned char buf [] = {255, 255, 255, 255, 0b00001111};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint<std::int32_t>(b);
			THEN("It is parsed successfully") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == -1);
				}
			}
		}
	}
}

SCENARIO("Varints may be parsed and have the result of that parse assigned to a variable", "[mcpp][protocol][varint]") {
	GIVEN("The representation of an integer") {
		unsigned char buf [] = {5};
		buffer b(buf);
		WHEN("It is parsed and the result of that parse is assigned to a variable") {
			int res;
			auto result = parse_varint(b, res);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The result is assigned to the variable") {
					CHECK(res == 5);
				}
			}
		}
	}
}

SCENARIO("A functor which parses varints may be created", "[mcpp][protocol][varint]") {
	GIVEN("A buffer containing the representation of two varints") {
		unsigned char buf [] = {0, 128, 1};	//	0, 128
		buffer b(buf);
		WHEN("A functor which parses from that buffer and assigns to an integer is created") {
			unsigned i;
			auto func = make_varint_parser(b, i);
			AND_WHEN("The functor is invoked") {
				auto result = func();
				THEN("The parse succeeds") {
					REQUIRE(result);
					AND_THEN("The first integer is parsed") {
						CHECK(i == 0);
					}
				}
				AND_WHEN("The functor is invoked again") {
					result = func();
					THEN("The parse succeeds") {
						REQUIRE(result);
						AND_THEN("The second integer is parsed") {
							CHECK(i == 128);
						}
					}
				}
			}
		}
	}
}

SCENARIO("Signed varints represented by ZigZag encoding may be parsed", "[mcpp][protocol][varint]") {
	GIVEN("The representation of zero") {
		unsigned char buf [] = {0};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint_zigzag<int>(b);
			THEN("It is parsed successfully") {
				REQUIRE(result);
				AND_THEN("Zero is parsed") {
					CHECK(result.value() == 0);
				}
			}
		}
	}
	GIVEN("A negative ZigZag representation") {
		unsigned char buf [] = {3};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint_zigzag<int>(b);
			THEN("It is parsed successfully") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == -2);
				}
			}
		}
	}
	GIVEN("A positive ZigZag representation") {
		unsigned char buf [] = {254, 255, 255, 255, 15};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_varint_zigzag<std::int32_t>(b);
			THEN("It is parsed successfully") {
				REQUIRE(result);
				AND_THEN("The correct integer is parsed") {
					CHECK(result.value() == std::numeric_limits<std::int32_t>::max());
				}
			}
		}
	}
}

SCENARIO("Signed varints represented by ZigZag encoding may be parsed and have the result of that parse assigned to a variable", "[mcpp][protocol][varint]") {
	GIVEN("The ZigZag representation of a signed integer") {
		unsigned char buf [] = {0};
		buffer b(buf);
		WHEN("It is parsed and the result of that parse is assigned to a variable") {
			int res;
			auto result = parse_varint_zigzag(b, res);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The result is assigned to the variable") {
					CHECK(res == 0);
				}
			}
		}
	}
}

SCENARIO("A functor which parses ZigZag encoded varints may be created", "[mcpp][protocol][varint]") {
	GIVEN("A buffer containing the ZigZag encoding of two varints") {
		unsigned char buf [] = {0, 128, 1};	//	0, 64
		buffer b(buf);
		WHEN("A functor which parses from that buffer and assigns to an integer is created") {
			int i;
			auto func = make_varint_zigzag_parser(b, i);
			AND_WHEN("The functor is invoked") {
				auto result = func();
				THEN("The parse succeeds") {
					REQUIRE(result);
					AND_THEN("The first integer is parsed") {
						CHECK(i == 0);
					}
				}
				AND_WHEN("The functor is invoked again") {
					result = func();
					THEN("The parse succeeds") {
						REQUIRE(result);
						AND_THEN("The second integer is parsed") {
							CHECK(i == 64);
						}
					}
				}
			}
		}
	}
}

SCENARIO("Unsigned varints may be written", "[mcpp][protocol][varint]") {
	GIVEN("A sufficiently large buffer") {
		unsigned char buf [5];
		buffer b(buf);
		WHEN("An integer whose varint representation is a single byte is serialized") {
			std::uint32_t u(0);
			serialize_varint(u, b);
			THEN("A single byte is written") {
				REQUIRE(b.written() == 1);
				AND_THEN("The correct byte is written") {
					CHECK(buf[0] == 0);
				}
			}
		}
		WHEN("An integer whose varint representation is multiple bytes is serialized") {
			serialize_varint(std::numeric_limits<std::uint32_t>::max() - 1, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 5);
				AND_THEN("The correct bytes are written") {
					CHECK(buf[0] == 254);
					CHECK(buf[1] == 255);
					CHECK(buf[2] == 255);
					CHECK(buf[3] == 255);
					CHECK(buf[4] == 15);
				}
			}
		}
	}
	GIVEN("An empty buffer") {
		buffer b;
		WHEN("An attempt is made to serialize a varint") {
			THEN("An exception is thrown") {
				CHECK_THROWS_AS(serialize_varint<unsigned>(0, b), write_overflow_error);
			}
		}
	}
}

SCENARIO("Signed varints may be written", "[mcpp][protocol][varint]") {
	GIVEN("A sufficiently large buffer") {
		unsigned char buf [5];
		buffer b(buf);
		WHEN("An integer whose varint representation is a single byte is serialized") {
			std::int32_t i(1);
			serialize_varint(i, b);
			THEN("A single byte is written") {
				REQUIRE(b.written() == 1);
				AND_THEN("The correct byte is written") {
					CHECK(buf[0] == 1);
				}
			}
		}
		WHEN("An integer whose varint representation is multiple bytes is serialized") {
			std::int32_t i(-1);
			serialize_varint(i, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 5);
				AND_THEN("The correct bytes are written") {
					CHECK(buf[0] == 255);
					CHECK(buf[1] == 255);
					CHECK(buf[2] == 255);
					CHECK(buf[3] == 255);
					CHECK(buf[4] == 15);
				}
			}
		}
	}
}

SCENARIO("ZigZag encoded signed varints may be written", "[mcpp][protocol][varint]") {
	GIVEN("A sufficiently large buffer") {
		unsigned char buf [3];
		buffer b(buf);
		WHEN("Zero is serialized") {
			std::int16_t i(0);
			serialize_varint_zigzag(i, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 1);
				AND_THEN("The correct byte is written") {
					CHECK(buf[0] == 0);
				}
			}
		}
		WHEN("A positive integer is serialized") {
			serialize_varint_zigzag(std::numeric_limits<std::int16_t>::max(), b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 3);
				AND_THEN("The correct bytes are written") {
					CHECK(buf[0] == 254);
					CHECK(buf[1] == 255);
					CHECK(buf[2] == 0b11);
				}
			}
		}
		WHEN("A negative integer is serialized") {
			serialize_varint_zigzag(std::numeric_limits<std::int16_t>::min(), b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 3);
				AND_THEN("The correct bytes are written") {
					CHECK(buf[0] == 255);
					CHECK(buf[1] == 255);
					CHECK(buf[2] == 0b11);
				}
			}
		}
	}
}

}
}
}
}
