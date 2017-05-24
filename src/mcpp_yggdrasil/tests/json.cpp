#include <mcpp/yggdrasil/json.hpp>
#include <mcpp/yggdrasil/error.hpp>
#include <mcpp/yggdrasil/agent.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/invalidate.hpp>
#include <mcpp/yggdrasil/profile.hpp>
#include <mcpp/yggdrasil/refresh.hpp>
#include <mcpp/yggdrasil/signout.hpp>
#include <mcpp/yggdrasil/user.hpp>
#include <mcpp/yggdrasil/validate.hpp>
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

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::refresh_request object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("A minimial mcpp::yggdrasil::refresh_request object") {
		refresh_request req("aoeui", "dhtns");
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"aoeui\","
					"\"clientToken\":\"dhtns\","
					"\"requestUser\":false"
				"}");
			}
		}
	}
	GIVEN("A maximal mcpp::yggdrasil::refresh_request object") {
		profile p("foo", "bar", true);
		refresh_request req("aoeui", "dhtns", std::move(p));
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"aoeui\","
					"\"clientToken\":\"dhtns\","
					"\"selectedProfile\":{"
						"\"id\":\"foo\","
						"\"name\":\"bar\","
						"\"legacy\":true"
					"},"
					"\"requestUser\":false"
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::refresh_request object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::refresh_request") {
		auto str = "{"
			"\"accessToken\":\"aoeui\","
			"\"clientToken\":\"dhtns\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<refresh_request>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && res = *result;
					CHECK(res.access_token == "aoeui");
					CHECK(res.client_token == "dhtns");
					CHECK_FALSE(res.selected_profile);
					CHECK_FALSE(res.request_user);
				}
			}
		}
	}
	GIVEN("A JSON string representing a maximal mcpp::yggdrasil::refresh_request") {
		auto str = "{"
			"\"accessToken\":\"aoeui\","
			"\"clientToken\":\"dhtns\","
			"\"selectedProfile\":{"
				"\"id\":\"foo\","
				"\"name\":\"bar\","
				"\"legacy\":true"
			"},"
			"\"requestUser\":false"
		"}";
		WHEN("It is parsed") {
			auto result = from_json<refresh_request>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && res = *result;
					CHECK(res.access_token == "aoeui");
					CHECK(res.client_token == "dhtns");
					REQUIRE(res.selected_profile);
					CHECK(res.selected_profile->id == "foo");
					CHECK(res.selected_profile->name == "bar");
					CHECK(res.selected_profile->legacy);
					CHECK_FALSE(res.request_user);
				}
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::refresh_response object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("A minimial mcpp::yggdrasil::refresh_response object") {
		refresh_response res("aoeui", "dhtns");
		WHEN("It is serialized to JSON") {
			auto str = to_json(res);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"aoeui\","
					"\"clientToken\":\"dhtns\""
				"}");
			}
		}
	}
	GIVEN("A maximal mcpp::yggdrasil::refresh_response object") {
		profile p("foo", "bar", true);
		user u("corge", {});
		u.properties["quux"] = "baz";
		refresh_response req("aoeui", "dhtns", std::move(p), std::move(u));
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"aoeui\","
					"\"clientToken\":\"dhtns\","
					"\"selectedProfile\":{"
						"\"id\":\"foo\","
						"\"name\":\"bar\","
						"\"legacy\":true"
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

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::refresh_response object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::refresh_response") {
		auto str = "{"
			"\"accessToken\":\"aoeui\","
			"\"clientToken\":\"dhtns\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<refresh_response>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && res = *result;
					CHECK(res.access_token == "aoeui");
					CHECK(res.client_token == "dhtns");
					CHECK_FALSE(res.selected_profile);
					CHECK_FALSE(res.user);
				}
			}
		}
	}
	GIVEN("A JSON string representing a maximal mcpp::yggdrasil::refresh_response") {
		auto str = "{"
			"\"accessToken\":\"aoeui\","
			"\"clientToken\":\"dhtns\","
			"\"selectedProfile\":{"
				"\"id\":\"foo\","
				"\"name\":\"bar\","
				"\"legacy\":true"
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
			auto result = from_json<refresh_response>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && res = *result;
					CHECK(res.access_token == "aoeui");
					CHECK(res.client_token == "dhtns");
					REQUIRE(res.selected_profile);
					CHECK(res.selected_profile->id == "foo");
					CHECK(res.selected_profile->name == "bar");
					CHECK(res.selected_profile->legacy);
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

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::api_error object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("A minimial mcpp::yggdrasil::api_error object") {
		api_error e("aoeui", "dhtns");
		WHEN("It is serialized to JSON") {
			auto str = to_json(e);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"error\":\"aoeui\","
					"\"errorMessage\":\"dhtns\""
				"}");
			}
		}
	}
	GIVEN("A maximal mcpp::yggdrasil::api_error object") {
		api_error e("aoeui", "dhtns", std::string("foo"));
		WHEN("It is serialized to JSON") {
			auto str = to_json(e);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"error\":\"aoeui\","
					"\"errorMessage\":\"dhtns\","
					"\"cause\":\"foo\""
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::api_error object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::api_error") {
		auto str = "{"
			"\"error\":\"aoeui\","
			"\"errorMessage\":\"dhtns\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<api_error>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && e = *result;
					CHECK(e.error == "aoeui");
					CHECK(e.error_message == "dhtns");
					CHECK_FALSE(e.cause);
				}
			}
		}
	}
	GIVEN("A JSON string representing a maximal mcpp::yggdrasil::api_error") {
		auto str = "{"
			"\"error\":\"aoeui\","
			"\"errorMessage\":\"dhtns\","
			"\"cause\":\"foo\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<api_error>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && e = *result;
					CHECK(e.error == "aoeui");
					CHECK(e.error_message == "dhtns");
					REQUIRE(e.cause);
					CHECK(*e.cause == "foo");
				}
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::validate_request object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("A minimial mcpp::yggdrasil::validate_request object") {
		validate_request req("quux");
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"quux\""
				"}");
			}
		}
	}
	GIVEN("A maximal mcpp::yggdrasil::validate_request object") {
		validate_request req("quux", std::string("corge"));
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"quux\","
					"\"clientToken\":\"corge\""
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::validate_request object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::validate_request") {
		auto str = "{"
			"\"accessToken\":\"quux\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<validate_request>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && req = *result;
					CHECK(req.access_token == "quux");
					CHECK_FALSE(req.client_token);
				}
			}
		}
	}
	GIVEN("A JSON string representing a maximal mcpp::yggdrasil::validate_request") {
		auto str = "{"
			"\"accessToken\":\"quux\","
			"\"clientToken\":\"corge\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<validate_request>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && req = *result;
					CHECK(req.access_token == "quux");
					REQUIRE(req.client_token);
					CHECK(*req.client_token == "corge");
				}
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::signout_request object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("A minimial mcpp::yggdrasil::signout_request object") {
		signout_request req("corge", "foo");
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"username\":\"corge\","
					"\"password\":\"foo\""
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::signout_request object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::signout_request") {
		auto str = "{"
			"\"username\":\"corge\","
			"\"password\":\"foo\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<signout_request>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && req = *result;
					CHECK(req.username == "corge");
					CHECK(req.password == "foo");
				}
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::to_json may be used to serialize an mcpp::yggdrasil::invalidate_request object to JSON", "[mcpp][yggdrasil][to_json]") {
	GIVEN("A minimial mcpp::yggdrasil::invalidate_request object") {
		invalidate_request req("bar", "baz");
		WHEN("It is serialized to JSON") {
			auto str = to_json(req);
			THEN("The correct JSON is returned") {
				CHECK(str == "{"
					"\"accessToken\":\"bar\","
					"\"clientToken\":\"baz\""
				"}");
			}
		}
	}
}

SCENARIO("mcpp::yggdrasil::from_json may be used to parse an mcpp::yggdrasil::invalidate_request object from JSON", "[mcpp][yggdrasil][from_json]") {
	GIVEN("A JSON string representing a minimal mcpp::yggdrasil::invalidate_request") {
		auto str = "{"
			"\"accessToken\":\"bar\","
			"\"clientToken\":\"baz\""
		"}";
		WHEN("It is parsed") {
			auto result = from_json<invalidate_request>(str);
			THEN("The parse succeeds") {
				REQUIRE(result);
				AND_THEN("The parsed object is correct") {
					auto && req = *result;
					CHECK(req.access_token == "bar");
					CHECK(req.client_token == "baz");
				}
			}
		}
	}
}

}
}
}
}
