#include <boost/system/error_code.hpp>
#include <mcpp/yggdrasil/http.hpp>
#include <sstream>
#include <string>

namespace mcpp {
namespace yggdrasil {
namespace detail {

const boost::system::error_category & http_error_category () {
	static const class final : public boost::system::error_category {
	public:
		virtual const char * name () const noexcept override {
			return "HTTP Status Code";
		}
		virtual std::string message (int condition) const override {
			switch (condition) {
			//	Taken from here: https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
			case 100:
				return "100 Continue";
			case 101:
				return "101 Switching Protocols";
			case 200:
				return "200 OK";
			case 201:
				return "201 Created";
			case 202:
				return "202 Accepted";
			case 203:
				return "203 Non-Authoritative Information";
			case 204:
				return "204 No Content";
			case 205:
				return "205 Reset Content";
			case 206:
				return "206 Partial Content";
			case 300:
				return "300 Multiple Choices";
			case 301:
				return "301 Moved Permanently";
			case 302:
				return "302 Found";
			case 303:
				return "303 See Other";
			case 304:
				return "304 Not Modified";
			case 305:
				return "305 Use Proxy";
			case 307:
				return "307 Temporary Redirect";
			case 400:
				return "400 Bad Request";
			case 401:
				return "401 Unauthorized";
			case 402:
				return "402 Payment Required";
			case 403:
				return "403 Forbidden";
			case 404:
				return "404 Not Found";
			case 405:
				return "405 Method Not Allowed";
			case 406:
				return "406 Not Acceptable";
			case 407:
				return "407 Proxy Authentication Required";
			case 408:
				return "408 Request Time-out";
			case 409:
				return "409 Conflict";
			case 410:
				return "410 Gone";
			case 411:
				return "411 Length Required";
			case 412:
				return "412 Precondition Failed";
			case 413:
				return "413 Request Entity Too Large";
			case 414:
				return "414 Request-URI Too Large";
			case 415:
				return "415 Unsupported Media Type";
			case 416:
				return "416 Requested range not satisfiable";
			case 417:
				return "417 Expectation Failed";
			case 500:
				return "500 Internal Server Error";
			case 501:
				return "501 Not Implemented";
			case 502:
				return "502 Bad Gateway";
			case 503:
				return "503 Service Unavailable";
			case 504:
				return "504 Gateway Time-out";
			case 505:
				return "505 HTTP Version not supported";
			default:
				break;
			}
			std::ostringstream ss;
			ss << condition;
			return ss.str();
		}
	} retr;
	return retr;
}

}
}
}
