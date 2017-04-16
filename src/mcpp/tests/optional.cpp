#include <mcpp/optional.hpp>
#include <type_traits>
#include <catch.hpp>

namespace mcpp {
namespace tests {
namespace {

SCENARIO("optional objects may be bound", "[mcpp][optional]") {
	GIVEN("Given zero optional objects and a functor") {
		bool invoked = false;
		auto f = [&] () noexcept {
			invoked = true;
			return 1;
		};
		WHEN("It is bound") {
			auto o = bind_optional(f);
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("An optional which holds a value is returned") {
				REQUIRE(o);
				AND_THEN("The optional holds the correct value") {
					CHECK(*o == 1);
				}
			}
		}
	}
	GIVEN("One optional object which holds a value and a functor") {
		bool invoked = false;
		auto f = [&] (auto i) noexcept {
			invoked = true;
			return i;
		};
		optional<int> m(5);
		WHEN("It is bound") {
			auto o = bind_optional(f, m);
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("An optional which holds a value is returned") {
				REQUIRE(o);
				AND_THEN("The optional holds the correct value") {
					CHECK(*o == 5);
				}
			}
		}
	}
	GIVEN("One optional object which does not hold a value and a functor") {
		bool invoked = false;
		auto f = [&] (auto i) noexcept {
			invoked = true;
			return i;
		};
		optional<int> m;
		WHEN("It is bound") {
			auto o = bind_optional(f, m);
			THEN("The functor is not invoked") {
				CHECK_FALSE(invoked);
			}
			THEN("An optional which does not hold a value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("Multiple optional objects which hold values and a functor") {
		bool invoked = false;
		auto f = [&] (int a, int b) noexcept {
			invoked = true;
			return a + b;
		};
		optional<int> a(2);
		optional<int> b(3);
		WHEN("It is bound") {
			auto o = bind_optional(f, a, b);
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("An optional which holds a value is returned") {
				REQUIRE(o);
				AND_THEN("The optional holds the correct value") {
					CHECK(*o == 5);
				}
			}
		}
	}
	GIVEN("Multiple optional objects at least one of which does not hold a value and a functor") {
		bool invoked = false;
		auto f = [&] (int a, int b) noexcept {
			invoked = true;
			return a + b;
		};
		optional<int> a(2);
		optional<int> b;
		WHEN("It is bound") {
			auto o = bind_optional(f, a, b);
			THEN("The functor is not invoked") {
				CHECK_FALSE(invoked);
			}
			THEN("An optional which does not hold a value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("An optional object which contains a value and at least one non-optional object") {
		bool invoked = false;
		auto f = [&] (int a, int b) noexcept {
			invoked = true;
			return a + b;
		};
		optional<int> a(2);
		int b = 3;
		WHEN("It is bound") {
			auto o = bind_optional(f, a, b);
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("An optional which holds a value is returned") {
				REQUIRE(o);
				AND_THEN("The optional holds the correct value") {
					CHECK(*o == 5);
				}
			}
		}
	}
	GIVEN("An optional object which does not contain a value and at least one non-optional object") {
		bool invoked = false;
		auto f = [&] (int a, int b) noexcept {
			invoked = true;
			return a + b;
		};
		optional<int> a;
		int b = 3;
		WHEN("It is bound") {
			auto o = bind_optional(f, a, b);
			THEN("The functor is not invoked") {
				CHECK_FALSE(invoked);
			}
			THEN("An optional which does not holds a value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("Nested optional objects which contain a value at all levels and a functor") {
		bool invoked = false;
		auto f = [&] (int a, int b) noexcept {
			invoked = true;
			return a + b;
		};
		optional<optional<int>> a(in_place, 5);
		optional<optional<optional<int>>> b(in_place, in_place, 5);
		WHEN("It is bound") {
			auto o = bind_optional(f, a, b);
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("An optional which holds a value is returned") {
				CHECK(o);
				AND_THEN("The optional holds the correct value") {
					CHECK(*o == 10);
				}
			}
		}
	}
	GIVEN("A nested optional object which does not contain a value at all levels and a functor") {
		bool invoked = false;
		auto f = [&] (int a) noexcept {
			invoked = true;
			return a;
		};
		optional<optional<optional<optional<int>>>> a(in_place, in_place);
		WHEN("It is bound") {
			auto o = bind_optional(f, a);
			THEN("The functor is not invoked") {
				CHECK_FALSE(invoked);
			}
			THEN("An optional which does not hold a value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("An optional which contains a value and a functor which returns an optional") {
		bool invoked = false;
		auto f = [&] (int a) noexcept -> optional<int> {
			invoked = true;
			return a;
		};
		optional<int> a(in_place, 5);
		WHEN("It is bound") {
			auto o = bind_optional(f, a);
			static_assert(std::is_same<decltype(o), optional<int>>::value, "Expected optional<int>");
			THEN("The functor is invoked") {
				CHECK(invoked);
			}
			THEN("An optional which holds a value is returned") {
				CHECK(o);
				AND_THEN("The optional holds the correct value") {
					CHECK(*o == 5);
				}
			}
		}
	}
}

}
}
}
