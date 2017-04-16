#include <mcpp/allocate_unique.hpp>
#include <mcpp/test/allocator.hpp>
#include <mcpp/test/object.hpp>
#include <exception>
#include <stdexcept>
#include <catch.hpp>

namespace mcpp {
namespace tests {
namespace {

SCENARIO("mcpp::allocate_unique may be used as a replacement for std::make_unique when an Allocator must be used", "[mcpp][allocate_unique]") {
	GIVEN("An allocator") {
		test::allocator_state state;
		test::allocator<test::object> a(state);
		test::object::state ostate;
		WHEN("A std::unique_ptr is created via mcpp::allocate_unique therewith") {
			auto ptr = allocate_unique<test::object>(a, ostate);
			THEN("The std::unique_ptr manages a pointee") {
				CHECK(ptr);
			}
			THEN("One object is allocated") {
				CHECK(state.allocations == 1);
				CHECK(state.allocated == sizeof(test::object));
			}
			THEN("One object is constructed") {
				CHECK(ostate.construct == 1);
			}
			AND_WHEN("The pointee is destroyed") {
				ptr.reset();
				THEN("One object is deallocated") {
					CHECK(state.deallocations == 1);
					CHECK(state.deallocated == sizeof(test::object));
				}
				THEN("One object is destroyed") {
					CHECK(ostate.destruct == 1);
				}
			}
		}
		WHEN("An attempt is made to create a std::unique_ptr via mcpp::allocate_unique, but the constructor of the pointee throws") {
			ostate.construct_exception = std::make_exception_ptr(std::runtime_error("Foo"));
			REQUIRE_THROWS_AS(allocate_unique<test::object>(a, ostate), std::runtime_error);
			THEN("One object is allocated") {
				CHECK(state.allocations == 1);
				CHECK(state.allocated == sizeof(test::object));
			}
			THEN("One object is deallocated") {
				CHECK(state.deallocations == 1);
				CHECK(state.deallocated == sizeof(test::object));
			}
		}
	}
}

}
}
}
