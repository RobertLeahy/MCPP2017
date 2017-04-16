#include <mcpp/iostreams/limiting_source.hpp>
#include <boost/core/ref.hpp>
#include <boost/iostreams/read.hpp>
#include <mcpp/buffer.hpp>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

SCENARIO("mcpp::iostreams::limiting_source restricts the number of characters which may be read from a Source", "[mcpp][iostreams][limiting_source]") {
	GIVEN("An mcpp::iostreams::limiting_source with a non-zero limit which wraps a Source which controls a sequence of zero characters") {
		buffer b;
		auto l = make_limiting_source(boost::ref(b), 10);
		WHEN("A read operation is performed") {
			char c;
			auto result = boost::iostreams::read(l, &c, 1);
			THEN("EOF is returned") {
				CHECK(result == -1);
			}
		}
	}
	GIVEN("An mcpp::iostreams::limiting_source with a non-zero limit which wraps a Source which controls a sequence of characters where the size of that sequence is lower than the limit") {
		char arr [] = {'a', 'b', 'c'};
		buffer b(arr);
		auto l = make_limiting_source(boost::ref(b), 10);
		WHEN("A read operation sufficient to drain the wrapped Source is performed") {
			char buffer [10];
			auto result = boost::iostreams::read(l, buffer, sizeof(buffer));
			THEN("The number of characters controlled by the wrapped Source are returned") {
				CHECK(result == sizeof(arr));
			}
			AND_WHEN("Another read operation is performed") {
				char c;
				auto result = boost::iostreams::read(l, &c, 1);
				THEN("EOF is returned") {
					CHECK(result == -1);
				}
			}
		}
	}
	GIVEN("An mcpp::iostreams::limiting_source with a non-zero limit which wraps a Source which controls a sequence of characters where the size of that sequence is higher than the limit") {
		char arr [] = {'a', 'b', 'c'};
		buffer b(arr);
		auto l = make_limiting_source(boost::ref(b), 2);
		WHEN("A read operation sufficient to drain the wrapped Source is performed") {
			char buffer [sizeof(arr)];
			auto result = boost::iostreams::read(l, buffer, sizeof(buffer));
			THEN("A number of characters equal to the limit are returned") {
				CHECK(result == 2);
			}
			AND_WHEN("Another read operation is performed") {
				char c;
				auto result = boost::iostreams::read(l, &c, 1);
				THEN("EOF is returned") {
					CHECK(result == -1);
				}
			}
		}
	}
	GIVEN("An mcpp::iostreams::limiting_source with a non-zero limit which wraps a Source which controls a non-empty sequence of characters") {
		char arr [] = {'a', 'b', 'c'};
		buffer b(arr);
		auto l = make_limiting_source(boost::ref(b), 0);
		WHEN("A read operation is performed") {
			char c;
			auto result = boost::iostreams::read(l, &c, 1);
			THEN("EOF is returned") {
				CHECK(result == -1);
			}
		}
	}
}

}
}
}
}
