#include <mcpp/iostreams/offset.hpp>
#include <mcpp/buffer.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <ios>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

class mock {
public:
	using category = void;
};

SCENARIO("mcpp::iostreams::offset may be used to obtain the offset of a stream", "[mcpp][iostreams][offset]") {
	GIVEN("A std::basic_streambuf which does not override seekoff") {
		unsigned char arr [5];
		buffer b(arr);
		WHEN("mcpp::iostreams::offset is invoked thereupon") {
			auto o = offset(b);
			THEN("No value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("A std::basic_streambuf which overrides seekoff") {
		char arr [5];
		boost::interprocess::bufferbuf b(arr, sizeof(arr));
		auto res = b.sputc('a');
		REQUIRE_FALSE(res == boost::interprocess::bufferbuf::traits_type::eof());
		WHEN("mcpp::iostreams::offset is invoked thereupon") {
			auto o = offset(b, std::ios_base::out);
			THEN("A value is returned") {
				REQUIRE(o);
				AND_THEN("The correct offset is returned") {
					CHECK(*o == 1);
				}
			}
		}
	}
	GIVEN("A device which is not a std::basic_istream, std::basic_ostream, or std::basic_streambuf and which is not seekable") {
		mock m;
		WHEN("mcpp::iostreams::offset is invoked thereupon") {
			auto o = offset(m);
			THEN("No value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
}

}
}
}
}
