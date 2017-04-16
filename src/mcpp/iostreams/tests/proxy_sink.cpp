#include <mcpp/iostreams/proxy_sink.hpp>
#include <boost/core/ref.hpp>
#include <boost/iostreams/write.hpp>
#include <mcpp/buffer.hpp>
#include <algorithm>
#include <iterator>
#include <catch.hpp>

namespace mcpp {
namespace iostreams {
namespace tests {
namespace {

SCENARIO("mcpp::iostreams::proxy_sink passes through write operations to the wrapped Sink", "[mcpp][iostreams][proxy_sink]") {
	GIVEN("An mcpp::iostreams::proxy_sink which wraps a Sink") {
		char buf [5];
		buffer b(buf);
		auto ps = make_proxy_sink(boost::ref(b));
		WHEN("A write operation is performed thereupon") {
			char data [] = {'a', 'b', 'c', 'd', 'e', 'f'};
			auto result = boost::iostreams::write(ps, data, sizeof(data));
			THEN("The correct result is returned") {
				REQUIRE(result == 5);
				AND_THEN("The write is performed") {
					using std::begin;
					using std::end;
					CHECK(std::equal(begin(buf), end(buf), begin(data), begin(data) + 5));
				}
			}
		}
	}
}

}
}
}
}
