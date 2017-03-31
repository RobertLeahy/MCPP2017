#include <mcpp/stream_log.hpp>
#include <sstream>
#include <catch.hpp>

namespace mcpp {
namespace test {
namespace {

SCENARIO("mcpp::stream_log writes messages which are not ignored to an underlying std::ostream","[mcpp][log][stream_log]") {
	GIVEN("An mcpp::stream_log") {
		std::ostringstream ss;
		stream_log log(ss);
		WHEN("A certain log level is ignored") {
			log.ignore(stream_log::level::info);
			AND_WHEN("A message of that level is logged") {
				log.write("test", "foo");
				THEN("Nothing is written to the underlying stream") {
					CHECK(ss.str().empty());
				}
			}
			AND_WHEN("A message of another level is logged") {
				log.write("baz", "corge", stream_log::level::debug);
				THEN("It is written to the underlying stream") {
					CHECK(ss.str() == "[DEBUG] [baz] corge\n");
				}
			}
		}
		WHEN("A message is logged") {
			log.write("test", "foo");
			THEN("It is written to the underlying stream") {
				CHECK(ss.str() == "[INFO] [test] foo\n");
			}
		}
	}
}

}
}
}
