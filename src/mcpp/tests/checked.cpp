#include <mcpp/checked.hpp>
#include <mcpp/optional.hpp>
#include <cstdint>
#include <limits>
#include <catch.hpp>

namespace mcpp {
namespace tests {
namespace {

SCENARIO("Checked conversions may be performed", "[mcpp][checked]") {
	GIVEN("A negative signed integer") {
		auto i = std::numeric_limits<std::int32_t>::min();
		WHEN("It is converted to an unsigned type") {
			auto o = checked::cast<unsigned>(i);
			THEN("The conversion is not performed") {
				CHECK_FALSE(o);
			}
		}
		WHEN("It is converted to a signed type which can represent the value") {
			auto o = checked::cast<std::int64_t>(i);
			THEN("The conversion is performed") {
				REQUIRE(o);
				AND_THEN("The converted value is correct") {
					CHECK(*o == i);
				}
			}
		}
		WHEN("It is converted to a signed type which cannot represent the value") {
			auto o = checked::cast<signed char>(i);
			THEN("The conversion is not performed") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("A positive signed integer") {
		auto i = std::numeric_limits<std::int32_t>::max();
		WHEN("It is converted to an unsigned type which can represent the value") {
			auto o = checked::cast<std::uint32_t>(i);
			THEN("The conversion is performed") {
				REQUIRE(o);
				AND_THEN("The converted value is correct") {
					CHECK(*o == i);
				}
			}
		}
		WHEN("It is converted to an unsigned type which cannot represent the value") {
			auto o = checked::cast<unsigned char>(i);
			THEN("The conversion is not performed") {
				CHECK_FALSE(o);
			}
		}
		WHEN("It is converted to a signed type which can represent the value") {
			auto o = checked::cast<std::int64_t>(i);
			THEN("The conversion is performed") {
				REQUIRE(o);
				AND_THEN("The converted value is correct") {
					CHECK(*o == i);
				}
			}
		}
		WHEN("It is converted to a signed type which cannot represent the value") {
			auto o = checked::cast<signed char>(i);
			THEN("The conversion is not performed") {
				CHECK_FALSE(o);
			}
		}
	}
	GIVEN("An unsigned integer") {
		auto i = std::numeric_limits<std::uint32_t>::max();
		WHEN("It is converted to an unsigned type which can represent the value") {
			auto o = checked::cast<std::uint64_t>(i);
			THEN("The conversion is performed") {
				REQUIRE(o);
				AND_THEN("The converted value is correct") {
					CHECK(*o == i);
				}
			}
		}
		WHEN("It is converted to an unsigned type which cannot represent the value") {
			auto o = checked::cast<unsigned char>(i);
			THEN("The conversion is not performed") {
				CHECK_FALSE(o);
			}
		}
		WHEN("It is converted to a signed type which can represent the value") {
			auto o = checked::cast<std::int64_t>(i);
			THEN("The conversion is performed") {
				REQUIRE(o);
				AND_THEN("The converted value is correct") {
					CHECK(*o == i);
				}
			}
		}
		WHEN("It is converted to a signed type which cannot represent the value") {
			auto o = checked::cast<signed char>(i);
			THEN("The conversion is not performed") {
				CHECK_FALSE(o);
			}
		}
	}
}

SCENARIO("mcpp::checked::cast accepts optional values", "[mcpp][checked]") {
	GIVEN("An optional which contains an integer") {
		optional<std::int32_t> i(in_place, 100000L);
		WHEN("It is converted and the conversion cannot be performed") {
			auto o = checked::cast<signed char>(i);
			THEN("An optional which does not contain a value is returned") {
				CHECK_FALSE(o);
			}
		}
		WHEN("It is converted and the conversion can be performed") {
			auto o = checked::cast<std::uint32_t>(i);
			THEN("An optional which contains a value is returned") {
				CHECK(o);
			}
		}
	}
	GIVEN("An optional which does not contain an integer") {
		optional<std::int32_t> i;
		WHEN("It is converted") {
			auto o = checked::cast<signed char>(i);
			THEN("An optional which does not contain a value is returned") {
				CHECK_FALSE(o);
			}
		}
	}
}

SCENARIO("Checked additions may be performed", "[mcpp][checked]") {
	GIVEN("No integers") {
		WHEN("mcpp::checked::add is invoked") {
			auto result = checked::add();
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("The result is correct") {
					CHECK(*result == 0);
				}
			}
		}
	}
	GIVEN("A single integer") {
		unsigned a = 10;
		WHEN("mcpp::checked::add is invoked") {
			auto result = checked::add(a);
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("The result is correct") {
					CHECK(*result == 10);
				}
			}
		}
	}
	GIVEN("Two unsigned integers which may safely be added") {
		unsigned a = 5;
		unsigned b = 2;
		WHEN("They are added with mcpp::checked::add") {
			auto result = checked::add(a, b);
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("The result is correct") {
					CHECK(*result == 7);
				}
			}
		}
	}
	GIVEN("Three unsigned integers which may not be safely added") {
		auto a = std::numeric_limits<unsigned>::max() - 1;
		unsigned b = 1;
		unsigned c = 1;
		WHEN("They are added with mcpp::checked::add") {
			auto result = checked::add(a, b, c);
			THEN("No result is returned") {
				CHECK_FALSE(result);
			}
		}
	}
}

SCENARIO("mcpp::checked::add accepts optional values", "[mcpp][checked]") {
	GIVEN("An optional which contains an integer") {
		optional<unsigned> i(5);
		WHEN("It is added to an integer and the addition would not overflow") {
			auto result = checked::add(i, 5);
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("That result is correct") {
					CHECK(*result == 10);
				}
			}
		}
		WHEN("It is added to an integer and the addition would overflow") {
			auto result = checked::add(i, std::numeric_limits<unsigned>::max() - 4);
			THEN("No result is returned") {
				CHECK_FALSE(result);
			}
		}
	}
	GIVEN("An optional which does not contain an integer") {
		optional<unsigned> i;
		WHEN("It is added to an integer") {
			auto result = checked::add(i, 1);
			THEN("No result is returned") {
				CHECK_FALSE(result);
			}
		}
	}
}

SCENARIO("Checked multiplications may be performed", "[mcpp][checked]") {
	GIVEN("No integers") {
		WHEN("mcpp::checked::multiply is invoked") {
			auto result = checked::multiply();
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("The result is correct") {
					CHECK(*result == 0);
				}
			}
		}
	}
	GIVEN("A single integer") {
		unsigned a = 10;
		WHEN("mcpp::checked::multiply is invoked") {
			auto result = checked::multiply(a);
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("The result is correct") {
					CHECK(*result == 10);
				}
			}
		}
	}
	GIVEN("Two unsigned integers which may safely be multiplied") {
		unsigned a = 5;
		unsigned b = 2;
		WHEN("They are multiplied with mcpp::checked::multiply") {
			auto result = checked::multiply(a, b);
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("The result is correct") {
					CHECK(*result == 10);
				}
			}
		}
	}
	GIVEN("Three unsigned integers which may not be safely multiplied") {
		auto a = std::numeric_limits<unsigned>::max() / 2;
		++a;
		unsigned b = 2;
		unsigned c = 1;
		WHEN("They are multiplied with mcpp::checked::multiply") {
			auto result = checked::multiply(a, b, c);
			THEN("No result is returned") {
				CHECK_FALSE(result);
			}
		}
	}
}

SCENARIO("mcpp::checked::multiply accepts optional values", "[mcpp][checked]") {
	GIVEN("An optional which contains an integer") {
		optional<unsigned> i(2);
		WHEN("It is multiplied by an integer and the multiplication would not overflow") {
			auto result = checked::multiply(i, 5);
			THEN("A result is returned") {
				REQUIRE(result);
				AND_THEN("That result is correct") {
					CHECK(*result == 10);
				}
			}
		}
		WHEN("It is multiplied by an integer and the multiplication would overflow") {
			auto result = checked::multiply(i, (std::numeric_limits<unsigned>::max() / 2) + 1);
			THEN("No result is returned") {
				CHECK_FALSE(result);
			}
		}
	}
	GIVEN("An optional which does not contain an integer") {
		optional<unsigned> i;
		WHEN("It is multiplied by an integer") {
			auto result = checked::multiply(i, 1);
			THEN("No result is returned") {
				CHECK_FALSE(result);
			}
		}
	}
}

}
}
}
