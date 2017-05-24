#include <mcpp/async.hpp>
#include <catch.hpp>

namespace mcpp {
namespace tests {
namespace {

class async : public mcpp::async {
public:
	~async () noexcept {
		stop();
	}
};

SCENARIO("A callback wrapped by mcpp::async may be invoked with effect prior to a call to mcpp::async::stop", "[mcpp][async]") {
	GIVEN("An mcpp::async object") {
		async a;
		WHEN("A callback is wrapped") {
			bool invoked = false;
			auto f = a.wrap([&] () noexcept {	invoked = true;	});
			AND_WHEN("The wrapper is invoked") {
				f();
				THEN("The wrapped callback is invoked") {
					CHECK(invoked);
				}
			}
		}
	}
}

SCENARIO("A callback wrapped by mcpp::async may not be invoked with effect after a call to mcpp::async::stop", "[mcpp][async]") {
	GIVEN("An mcpp::async object") {
		async a;
		WHEN("A callback is wrapped") {
			bool invoked = false;
			auto f = a.wrap([&] () noexcept {	invoked = true;	});
			AND_WHEN("mcpp::async::stop is invoked") {
				a.stop();
				AND_WHEN("The wrapper is invoked") {
					f();
					THEN("The wrapped callback is not invoked") {
						CHECK_FALSE(invoked);
					}
				}
			}
		}
	}
}

}
}
}
