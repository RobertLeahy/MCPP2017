#include <mcpp/yggdrasil/http.hpp>
#include <beast/core/error.hpp>
#include <beast/core/flat_buffer.hpp>
#include <beast/http/fields.hpp>
#include <beast/test/string_iostream.hpp>
#include <boost/asio/io_service.hpp>
#include <mcpp/optional.hpp>
#include <mcpp/variant.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <utility>
#include <catch.hpp>

namespace mcpp {
namespace yggdrasil {
namespace tests {
namespace {

SCENARIO("Yggdrasil REST requests may be made vie AsyncStream", "[mcpp][yggdrasil][http]") {
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
		optional<variant<beast::error_code, authenticate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result = std::move(ec);
			else result = std::move(response);
		};
		WHEN("An authenticate request is submitted") {
			authenticate_request req("baz", "quux");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			while (!result) io_service.poll_one();
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
	GIVEN("A model of AsyncStream which yields an HTTP response containing a status other than 200 OK") {
		boost::asio::io_service io_service;
		beast::test::string_iostream ios(
			io_service,
			"HTTP/1.1 403 \r\n\r\n"
		);
		beast::flat_buffer buffer;
		optional<variant<beast::error_code, authenticate_response>> result;
		auto handler = [&] (auto ec, auto response) {
			if (ec) result = std::move(ec);
			else result = std::move(response);
		};
		WHEN("An authenticate request is submitted") {
			authenticate_request req("corge", "bar");
			async_http_request(ios, buffer, std::move(req), beast::http::fields{}, handler);
			while (!result) io_service.poll_one();
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
				auto && ec = get<beast::error_code>(*result);
				CHECK(ec.value() == 403);
				CHECK(ec.message() == "403 Forbidden");
			}
		}
	}
}

}
}
}
}
