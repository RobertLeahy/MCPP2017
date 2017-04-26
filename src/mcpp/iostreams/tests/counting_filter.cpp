#include <mcpp/iostreams/counting_filter.hpp>
#include <boost/core/ref.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <mcpp/buffer.hpp>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

SCENARIO("mcpp::iostreams::counting_filter counts bytes written through it", "[mcpp][iostreams][counting_filter]") {
	GIVEN("An mcpp::iostreams::counting_filter object") {
		counting_filter filter;
		unsigned char buf [16];
		buffer be(buf);
		//auto sink = boost::iostreams::compose(filter, boost::ref(b));
		WHEN("Characters are written through the filter") {
			unsigned char buf [] = {1, 2, 3, 4};
			buffer b(buf);
			boost::iostreams::copy(b, be);
			THEN("The count is correct") {
				//CHECK(filter.count() == sizeof(buf));
			}
		}
	}
}

}
}
}
}
