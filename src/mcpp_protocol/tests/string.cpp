#include <mcpp/protocol/string.hpp>
#include <mcpp/buffer.hpp>
#include <mcpp/protocol/error.hpp>
#include <mcpp/protocol/exception.hpp>
#include <algorithm>
#include <iterator>
#include <string>
#include <catch.hpp>

namespace mcpp {
namespace protocol {
namespace tests {
namespace {

SCENARIO("Strings may be parsed", "[mcpp][protocol][string]") {
	GIVEN("An empty buffer") {
		buffer b;
		WHEN("It is parsed") {
			auto result = parse_string(b);
			THEN("The parse is unsuccessful") {
				REQUIRE_FALSE(result);
				auto && e = result.error();
				AND_THEN("The correct error code is returned") {
					CHECK(e == make_error_code(error::end_of_file));
				}
			}
		}
	}
	GIVEN("A buffer containing the representation of the empty string") {
		unsigned char buf [] = {0};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_string(b);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The empty string is parsed") {
					CHECK(result->empty());
				}
			}
		}
	}
	GIVEN("A buffer containing the representation of a string") {
		unsigned char buf [] = {3, 'f', 'o', 'o'};
		buffer b(buf);
		WHEN("It is parsed") {
			auto result = parse_string(b);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The correct string is parsed") {
					CHECK(*result == "foo");
				}
			}
		}
		WHEN("It is parsed as UTF-16") {
			auto result = parse_string<char16_t>(b);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The correct string is parsed") {
					auto && str = *result;
					REQUIRE(str.size() == 3);
					CHECK(str[0] == 'f');
					CHECK(str[1] == 'o');
					CHECK(str[2] == 'o');
				}
			}
		}
	}
}

SCENARIO("Strings may be parsed and the result thereof may be assigned to a provided string", "[mcpp][protocol][string]") {
	GIVEN("A buffer containing the representation of a string") {
		unsigned char buf [] = {3, 'f', 'o', 'o'};
		buffer b(buf);
		WHEN("The buffer is parsed") {
			std::string str;
			auto result = parse_string(b, str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed string is assigned to the provided string") {
					CHECK(str == "foo");
				}
			}
		}
	}
}

SCENARIO("Functors which parse strings may be created", "[mcpp][protocol][string]") {
	GIVEN("A buffer containing the representation of two strings") {
		unsigned char buf [] = {
			3, 'f', 'o', 'o',
			4, 'q', 'u', 'u', 'x'
		};
		buffer b(buf);
		WHEN("A functor which parses from that buffer into a provided string is created") {
			std::string res;
			auto func = make_string_parser(b, res);
			AND_WHEN("That functor is invoked") {
				auto result = func();
				THEN("The parse succeeds") {
					REQUIRE(result);
					AND_THEN("The first string is parsed") {
						CHECK(res == "foo");
					}
				}
				AND_WHEN("That functor is invoked again") {
					result = func();
					THEN("The parse succeeds") {
						REQUIRE(result);
						AND_THEN("The second string is parsed") {
							CHECK(res == "quux");
						}
					}
				}
			}
		}
	}
}

SCENARIO("Strings may be serialized", "[mcpp][protocol][string]") {
	GIVEN("An empty string") {
		std::string str;
		unsigned char buf [1];
		buffer b(buf);
		WHEN("It is serialized") {
			serialize_string(str, b);
			THEN("One byte is written") {
				REQUIRE(b.written() == 1);
				AND_THEN("The correct byte is written") {
					CHECK(buf[0] == 0);
				}
			}
		}
	}
	GIVEN("A non-empty string") {
		std::string str("hello");
		char buf [6];
		buffer b(buf);
		WHEN("It is serialized") {
			serialize_string(str, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 6);
				AND_THEN("The correct bytes are written") {
					char expected [] = {5, 'h', 'e', 'l', 'l', 'o'};
					using std::begin;
					using std::end;
					CHECK(std::equal(begin(buf), end(buf), begin(expected), end(expected)));
				}
			}
		}
	}
	GIVEN("A non-empty string and a buffer of insufficient size to hold it's serialized representaiton") {
		std::string str("hello");
		char buf [5];
		buffer b(buf);
		WHEN("It is serialized") {
			THEN("An exception is thrown") {
				CHECK_THROWS_AS(serialize_string(str, b), write_overflow_error);
			}
		}
	}
	GIVEN("A UTF-16 string") {
		std::u16string str(u"hello");
		char buf [6];
		buffer b(buf);
		WHEN("It is serialized") {
			serialize_string(str, b);
			THEN("The correct number of bytes are written") {
				REQUIRE(b.written() == 6);
				AND_THEN("The correct bytes are written") {
					char expected [] = {5, 'h', 'e', 'l', 'l', 'o'};
					using std::begin;
					using std::end;
					CHECK(std::equal(begin(buf), end(buf), begin(expected), end(expected)));
				}
			}
		}
	}
}

}
}
}
}
