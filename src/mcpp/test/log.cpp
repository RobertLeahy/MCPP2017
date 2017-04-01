#include <mcpp/log.hpp>
#include <mcpp/log_level.hpp>
#include <mcpp/stream_log.hpp>
#include <sstream>
#include <catch.hpp>

namespace mcpp {
namespace test {
namespace {

SCENARIO("mcpp::log::write may be provided with a functor to lazily generate a log message","[mcpp][log]") {
	GIVEN("An instance of a class which derives from mcpp::log") {
		std::ostringstream ss;
		stream_log log(ss);
		bool invoked = false;
		auto f = [&] () {
			invoked = true;
			return "Hello world";
		};
		WHEN("A message is written to the log by providing a functor which lazily generates a log message") {
			log.write("test", f);
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("The message is written to the log") {
				CHECK(ss.str() == "[INFO] [test] Hello world\n");
			}
		}
		WHEN("A certain log level is ignored") {
			log.ignore(log_level::info);
			AND_WHEN("A message is written to the log at that level by providing a functor which lazily generates a log message") {
				log.write("test", f);
				THEN("The functor is not invoked") {
					CHECK_FALSE(invoked);
				}
				THEN("The message is not written to the log") {
					CHECK(ss.str().empty());
				}
			}
			AND_WHEN("A message is written to the log at a different level by providing a functor which lazily generates a log message") {
				log.write("test", f, log_level::debug);
				THEN("The functor is invoked") {
					CHECK(invoked);
				}
				THEN("The message is written to the log") {
					CHECK(ss.str() == "[DEBUG] [test] Hello world\n");
				}
			}
		}
	}
}

}
}
}
