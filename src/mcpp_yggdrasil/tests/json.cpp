#include <mcpp/yggdrasil/json.hpp>
#include <mcpp/yggdrasil/agent.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/profile.hpp>
#include <mcpp/yggdrasil/user.hpp>
#include <string>
#include <utility>
#include <catch.hpp>

namespace mcpp {
namespace yggdrasil {
namespace tests {
namespace {

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::authenticate_request object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("An mcpp::yggdrasil::authenticate_request with none of its optional members") {
		authenticate_request request("foo", "bar");
		WHEN("It is serialized to JSON") {
			auto json = to_json(request);
			THEN("The correct JSON is returned") {
				CHECK(json == "{"
					"\"username\":\"foo\","
					"\"password\":\"bar\","
					"\"requestUser\":false"
				"}");
			}
		}
	}
	GIVEN("An mcpp::yggdrasil::authenticate_request with all of its optional members") {
		agent a("baz", 2);
		authenticate_request request("foo", "corge", std::move(a), std::string("quux"), true);
		WHEN("It is serialized to JSON") {
			auto json = to_json(request);
			THEN("The correct JSON is returned") {
				CHECK(json == "{"
					"\"agent\":{"
						"\"name\":\"baz\","
						"\"version\":2"
					"},"
					"\"username\":\"foo\","
					"\"password\":\"corge\","
					"\"clientToken\":\"quux\","
					"\"requestUser\":true"
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::authenticate_request object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::authenticate_request") {
		auto str = "{"
			"\"username\":\"foo\","
			"\"password\":\"bar\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<authenticate_request>(str);
			THEN("The parse is successful") {
				REQUIRE(result);
				AND_THEN("A correct mcpp::yggdrasil::authenticate_request is returned") {
					auto && req = *result;
					CHECK_FALSE(req.agent);
					CHECK(req.username == "foo");
					CHECK(req.password == "bar");
					CHECK_FALSE(req.client_token);
					CHECK_FALSE(req.request_user);
				}
			}
		}
	}
	GIVEN("A JSON string representing a maximal mcpp::yggdrasil::authenticate_request") {
		auto str = "{"
			"\"agent\":{"
				"\"name\":\"baz\","
				"\"version\":2"
			"},"
			"\"username\":\"foo\","
			"\"password\":\"corge\","
			"\"clientToken\":\"quux\","
			"\"requestUser\":true"
		"}";
		WHEN("It is parsed") {
			auto result = from_json<authenticate_request>(str);
			THEN("The parse is successful") {
				REQUIRE(result);
				AND_THEN("A correct mcpp::yggdrasil::authenticate_request is returned") {
					auto && req = *result;
					REQUIRE(req.agent);
					CHECK(req.agent->name == "baz");
					CHECK(req.agent->version == 2);
					CHECK(req.username == "foo");
					CHECK(req.password == "corge");
					REQUIRE(req.client_token);
					CHECK(*req.client_token == "quux");
					CHECK(req.request_user);
				}
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::authenticate_response object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("An mcpp::yggdrasil::authenticate_response with none of its optional members") {
		authenticate_response response("foo", "bar");
		WHEN("It is serialized to JSON") {
			auto json = to_json(response);
			THEN("The correct JSON is returned") {
				CHECK(json == "{"
					"\"accessToken\":\"foo\","
					"\"clientToken\":\"bar\""
				"}");
			}
		}
	}
	GIVEN("An mcpp::yggdrasil::authenticate_response with all of its optional members") {
		user u("corge", {});
		u.properties["quux"] = "baz";
		profile p("abc", "def", false);
		authenticate_response response(
			"foo",
			"bar",
			authenticate_response::available_profiles_type{},
			std::move(p),
			std::move(u)
		);
		WHEN("It is serialized to JSON") {
			auto json = to_json(response);
			THEN("The correct JSON is returned") {
				CHECK(json == "{"
					"\"accessToken\":\"foo\","
					"\"clientToken\":\"bar\","
					"\"availableProfiles\":[],"
					"\"selectedProfile\":{"
						"\"id\":\"abc\","
						"\"name\":\"def\","
						"\"legacy\":false"
					"},"
					"\"user\":{"
						"\"id\":\"corge\","
						"\"properties\":["
							"{"
								"\"name\":\"quux\","
								"\"value\":\"baz\""
							"}"
						"]"
					"}"
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::authenticate_response object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::authenticate_response") {
		auto str = "{"
			"\"accessToken\":\"foo\","
			"\"clientToken\":\"bar\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<authenticate_response>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && res = *result;
					CHECK(res.access_token == "foo");
					CHECK(res.client_token == "bar");
					CHECK_FALSE(res.available_profiles);
					CHECK_FALSE(res.selected_profile);
					CHECK_FALSE(res.user);
				}
			}
		}
	}
	GIVEN("A JSON string representing a maximal mcpp::yggdrasil::authenticate_response") {
		auto str = "{"
			"\"accessToken\":\"foo\","
			"\"clientToken\":\"bar\","
			"\"availableProfiles\":[],"
			"\"selectedProfile\":{"
				"\"id\":\"abc\","
				"\"name\":\"def\","
				"\"legacy\":false"
			"},"
			"\"user\":{"
				"\"id\":\"corge\","
				"\"properties\":["
					"{"
						"\"name\":\"quux\","
						"\"value\":\"baz\""
					"}"
				"]"
			"}"
		"}";
		WHEN("It is parsed") {
			auto result = from_json<authenticate_response>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && res = *result;
					CHECK(res.access_token == "foo");
					CHECK(res.client_token == "bar");
					REQUIRE(res.available_profiles);
					CHECK(res.available_profiles->empty());
					REQUIRE(res.selected_profile);
					CHECK(res.selected_profile->id == "abc");
					CHECK(res.selected_profile->name == "def");
					CHECK_FALSE(res.selected_profile->legacy);
					REQUIRE(res.user);
					CHECK(res.user->id == "corge");
					REQUIRE(res.user->properties.size() == 1);
					auto iter = res.user->properties.begin();
					CHECK(iter->first == "quux");
					CHECK(iter->second == "baz");
				}
			}
		}
	}
}

}
}
}
}
