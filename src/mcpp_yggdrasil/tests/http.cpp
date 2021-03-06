#include <mcpp/yggdrasil/http.hpp>
#include "config.hpp"
#include <beast/core/error.hpp>
#include <beast/core/flat_buffer.hpp>
#include <beast/http/fields.hpp>
#include <beast/test/string_iostream.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <mcpp/optional.hpp>
#include <mcpp/variant.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/error.hpp>
#include <mcpp/yggdrasil/invalidate.hpp>
#include <mcpp/yggdrasil/json.hpp>
#include <mcpp/yggdrasil/refresh.hpp>
#include <mcpp/yggdrasil/signout.hpp>
#include <mcpp/yggdrasil/validate.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <catch.hpp>

namespace mcpp {
namespace yggdrasil {
namespace tests {
namespace {

#if defined(MCPP_YGGDRASIL_USERNAME) && defined(MCPP_YGGDRASIL_PASSWORD)
SCENARIO("Requests may be made against the actual Yggdrasil API", "[mcpp][yggdrasil][http][.][integration]") {
	GIVEN("A Yggdrasil request, Asio socket, Asio SSL stream, and Asio resolver") {
		boost::asio::io_service io_service;
		beast::flat_buffer buffer;
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
		ctx.set_verify_mode(boost::asio::ssl::verify_none);
		using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
		stream_type stream(io_service, ctx);
		boost::asio::deadline_timer timer(io_service);
		class timeout {	};
		optional<variant<error, authenticate_response, timeout>> result;
		timer.expires_from_now(boost::posix_time::seconds(5));
		timer.async_wait([&] (auto) noexcept {	result.emplace(in_place_type<timeout>);	});
		WHEN("\"authserver.mojang.com\" is resolved") {
			boost::asio::ip::tcp::resolver::query q("authserver.mojang.com", "https");
			optional<variant<beast::error_code, optional<boost::asio::ip::tcp::endpoint>>> ep;
			resolver.async_resolve(q, [&] (auto ec, auto iter) {
				if (ec) {
					ep.emplace(ec);
					return;
				}
				optional<boost::asio::ip::tcp::endpoint> local;
				if (iter != decltype(iter){}) local.emplace(*iter);
				ep.emplace(local);
			});
			do io_service.run_one();
			while (!result && !ep);
			if (result) FAIL("Timed out");
			auto ec = get_if<beast::error_code>(&*ep);
			if (ec) FAIL(ec->message());
			auto opt_ep = get<optional<boost::asio::ip::tcp::endpoint>>(*ep);
			if (!opt_ep) FAIL("No endpoints for \"authserver.mojang.com\"");
			AND_WHEN("A connection is formed thereto") {
				optional<beast::error_code> ec;
				stream.lowest_layer().async_connect(*opt_ep, [&] (auto local) {	ec = local;	});
				do io_service.run_one();
				while (!result && !ec);
				if (result) FAIL("Timed out");
				if (*ec) FAIL(ec->message());
				AND_WHEN("The SSL handshake is performed") {
					ec = nullopt;
					stream.async_handshake(stream_type::client, [&] (auto local) {	ec = local;	});
					do io_service.run_one();
					while (!result && !ec);
					if (result) FAIL("Timed out");
					if (*ec) FAIL(ec->message());
					AND_WHEN("A Yggdrasil request is made") {
						authenticate_request request(MCPP_YGGDRASIL_USERNAME, MCPP_YGGDRASIL_PASSWORD);
						beast::http::fields fields;
						fields.replace("Host", "authserver.mojang.com");
						async_http_request(stream, buffer, std::move(request), std::move(fields), [&] (auto ec, auto res) {
							if (ec) {
								result.emplace(ec);
								return;
							}
							result.emplace(std::move(res));
						});
						do io_service.run_one();
						while (!result);
						THEN("The response is received successfully") {
							if (get_if<timeout>(&*result)) FAIL("Timed out");
							auto ep = get_if<error>(&*result);
							if (ep) {
								std::ostringstream ss;
								ss << ep->message();
								if (ep->api) {
									ss << " - " << ep->api->error << " - " << ep->api->error_message;
									if (ep->api->cause) ss << " - " << *ep->api->cause;
								}
								FAIL(ss.str());
							}
							INFO(to_json(get<authenticate_response>(*result)));
							CHECK(get_if<authenticate_response>(&*result));
						}
					}
				}
			}
		}
	}
}
#endif

SCENARIO("Authenticate REST requests may be made via AsyncStream", "[mcpp][yggdrasil][http]") {
	GIVEN("A model of AsyncStream which yields the HTTP response to an authenticate request") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json; charset=utf8\r\n"
			"Content-Length: 41\r\n"
			"\r\n"
			"{\"accessToken\":\"foo\",\"clientToken\":\"bar\"}"
		);
		beast::flat_buffer buffer;
		optional<variant<error, authenticate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result.emplace(std::move(ec));
			else result.emplace(std::move(response));
		};
		WHEN("An authenticate request is submitted") {
			authenticate_request req("baz", "quux");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /authenticate HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 56\r\n"
					"\r\n"
					"{\"username\":\"baz\",\"password\":\"quux\",\"requestUser\":false}"
				);
			}
			THEN("The correct response is successfully parsed") {
				auto && res = get<authenticate_response>(*result);
				CHECK(res.access_token == "foo");
				CHECK(res.client_token == "bar");
				CHECK_FALSE(res.available_profiles);
				CHECK_FALSE(res.selected_profile);
				CHECK_FALSE(res.user);
			}
		}
	}
}

SCENARIO("Refresh requests may be made via AsyncStream", "[mcpp][yggdrasil][http]") {
	GIVEN("A model of AsyncStream which yields the HTTP response to a refresh request") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json; charset=utf8\r\n"
			"Content-Length: 41\r\n"
			"\r\n"
			"{\"accessToken\":\"foo\",\"clientToken\":\"bar\"}"
		);
		beast::flat_buffer buffer;
		optional<variant<error, refresh_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result.emplace(std::move(ec));
			else result.emplace(std::move(response));
		};
		WHEN("A refresh request is submitted") {
			refresh_request req("baz", "quux");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /refresh HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 62\r\n"
					"\r\n"
					"{\"accessToken\":\"baz\",\"clientToken\":\"quux\",\"requestUser\":false}"
				);
			}
			THEN("The correct response is successfully parsed") {
				auto && res = get<refresh_response>(*result);
				CHECK(res.access_token == "foo");
				CHECK(res.client_token == "bar");
				CHECK_FALSE(res.selected_profile);
				CHECK_FALSE(res.user);
			}
		}
	}
}

SCENARIO("Validate requests may be made via AsyncStream", "[mcpp][yggdrasil][http]") {
	GIVEN("A model of AsyncStream which yields the HTTP response to a successful validate request") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 204 No Content\r\n\r\n"
		);
		beast::flat_buffer buffer;
		optional<variant<error, validate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result.emplace(std::move(ec));
			else result.emplace(std::move(response));
		};
		WHEN("A validate request is submitted") {
			validate_request req("corge");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /validate HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 23\r\n"
					"\r\n"
					"{\"accessToken\":\"corge\"}"
				);
			}
			THEN("The correct response is successfully parsed") {
				CHECK(get<validate_response>(*result));
			}
		}
	}
	GIVEN("A model of AsyncStream which yields the HTTP response to an unsuccessful validate request") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 403 Forbidden\r\n\r\n"
		);
		beast::flat_buffer buffer;
		optional<variant<error, validate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result.emplace(std::move(ec));
			else result.emplace(std::move(response));
		};
		WHEN("A validate request is submitted") {
			validate_request req("corge", std::string("quux"));
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /validate HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 44\r\n"
					"\r\n"
					"{\"accessToken\":\"corge\",\"clientToken\":\"quux\"}"
				);
			}
			THEN("The correct response is successfully parsed") {
				CHECK(!get<validate_response>(*result));
			}
		}
	}
}

SCENARIO("Signout requests may be made via AsyncStream", "[mcpp][yggdrasil][http]") {
	GIVEN("A model of AsyncStream which yields the HTTP response to a successful signout request") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 204 No Content\r\n\r\n"
		);
		beast::flat_buffer buffer;
		optional<error> result;
		auto handler = [&] (auto ec) {	result.emplace(ec);	};
		WHEN("A validate request is submitted") {
			signout_request req("foo", "bar");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /signout HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 35\r\n"
					"\r\n"
					"{\"username\":\"foo\",\"password\":\"bar\"}"
				);
			}
			THEN("The correct response is successfully parsed") {
				CHECK_FALSE(*result);
			}
		}
	}
}

SCENARIO("Invalidate requests may be made via AsyncStream", "[mcpp][yggdrasil][http]") {
	GIVEN("A model of AsyncStream which yields the HTTP response to a successful invalidate request") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 204 No Content\r\n\r\n"
		);
		beast::flat_buffer buffer;
		optional<error> result;
		auto handler = [&] (auto ec) {	result.emplace(ec);	};
		WHEN("A validate request is submitted") {
			invalidate_request req("quux", "corge");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /invalidate HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 44\r\n"
					"\r\n"
					"{\"accessToken\":\"quux\",\"clientToken\":\"corge\"}"
				);
			}
			THEN("The correct response is successfully parsed") {
				CHECK_FALSE(*result);
			}
		}
	}
}

SCENARIO("REST requests which end in error are handled appropriately", "[mcpp][yggdrasil][http]") {
	GIVEN("A model of AsyncStream which yields an HTTP response containing a non-2xx status") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 403 \r\n\r\n"
		);
		beast::flat_buffer buffer;
		optional<variant<error, authenticate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result.emplace(std::move(ec));
			else result.emplace(std::move(response));
		};
		WHEN("An authenticate request is submitted") {
			authenticate_request req("corge", "bar");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /authenticate HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 57\r\n"
					"\r\n"
					"{\"username\":\"corge\",\"password\":\"bar\",\"requestUser\":false}"
				);
			}
			THEN("The asynchronous operation ends in error") {
				auto && ec = get<error>(*result);
				CHECK(ec.value() == 403);
				CHECK(ec.message() == "403 Forbidden");
				CHECK_FALSE(ec.api);
			}
		}
	}
	GIVEN("A model of AsyncStream which yields an HTTP response containing a non-2xx status and a body which represents a Yggdrasil error") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 500 \r\n"
			"Content-Type: application/json; charset=utf-8\r\n"
			"Length: 36\r\n"
			"\r\n"
			"{\"error\":\"foo\",\"errorMessage\":\"baz\"}"
		);
		beast::flat_buffer buffer;
		optional<variant<error, authenticate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result.emplace(std::move(ec));
			else result.emplace(std::move(response));
		};
		WHEN("An authenticate request is submitted") {
			authenticate_request req("corge", "bar");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			do io_service.run_one();
			while (!result);
			THEN("The correct HTTP request is generated") {
				CHECK(ios.str ==
					"POST /authenticate HTTP/1.1\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Content-Length: 57\r\n"
					"\r\n"
					"{\"username\":\"corge\",\"password\":\"bar\",\"requestUser\":false}"
				);
			}
			THEN("The asynchronous operation ends in the appropriacet Yggdrasil error") {
				auto && ec = get<error>(*result);
				CHECK(ec.value() == 500);
				CHECK(ec.message() == "500 Internal Server Error");
				REQUIRE(ec.api);
				CHECK(ec.api->error == "foo");
				CHECK(ec.api->error_message == "baz");
				CHECK_FALSE(ec.api->cause);
			}
		}
	}
}

}
}
}
}
