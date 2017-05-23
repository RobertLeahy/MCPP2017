/**
 *	\file
 */

#pragma once

#include "authenticate.hpp"
#include "error.hpp"
#include "json.hpp"
#include "refresh.hpp"
#include "signout.hpp"
#include "validate.hpp"
#include <beast/core/async_result.hpp>
#include <beast/core/error.hpp>
#include <beast/core/handler_alloc.hpp>
#include <beast/core/handler_ptr.hpp>
#include <beast/http/fields.hpp>
#include <beast/http/message.hpp>
#include <beast/http/message_parser.hpp>
#include <beast/http/read.hpp>
#include <beast/http/string_body.hpp>
#include <beast/http/write.hpp>
#include <boost/asio/handler_alloc_hook.hpp>
#include <boost/asio/handler_continuation_hook.hpp>
#include <boost/asio/handler_invoke_hook.hpp>
#include <boost/expected/expected.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/optional.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mcpp {
namespace yggdrasil {

namespace detail {

template <typename Response>
class response_base {
public:
	using type = Response;
};

template <typename Request>
class response_helper;
template <>
class response_helper<authenticate_request> : public response_base<authenticate_response> {	};
template <>
class response_helper<refresh_request> : public response_base<refresh_response> {	};
template <>
class response_helper<validate_request> : public response_base<validate_response> {	};
template <>
class response_helper<signout_request> : public response_base<signout_response> {	};

template <typename Request>
using response_t = typename response_helper<Request>::type;

const boost::system::error_category & http_error_category ();

template <typename Fields>
boost::unexpected_type<error> parse_error (const beast::http::response<beast::http::string_body, Fields> & response) {
	error e(beast::error_code(response.status, http_error_category()));
	auto result = from_json<api_error>(response.body);
	if (result) e.api = std::move(*result);
	return boost::make_unexpected(std::move(e));
}

class void_parser {
public:
	using response_type = void;
	using result_type = boost::expected<void, error>;
	template <typename Fields>
	static result_type parse (const beast::http::response<beast::http::string_body, Fields> & response) {
		if (response.status != 204) return parse_error(response);
		//	The expected library is broken apparently so
		//	neither of these work:
		//
		//	return result_type{};
		//	return boost::make_expected<error>();
		//
		//	Even though best I can tell the propasal says
		//	either should work
		boost::expected<int, error> workaround(0);
		return workaround.map([] (auto) noexcept {	});
	}
};

template <typename Request>
class parse_response {
public:
	using response_type = response_t<Request>;
	using result_type = boost::expected<response_type, error>;
	template <typename Fields>
	static result_type parse (const beast::http::response<beast::http::string_body, Fields> & response) {
		if (response.status != 200) return parse_error(response);
		auto result = from_json<response_type>(response.body);
		if (!result) {
			auto && ec = result.error();
			assert(ec.category() == from_json_error_category());
			return boost::make_unexpected(
				error(
					beast::error_code(
						ec.value(),
						from_json_boost_error_category()
					)
				)
			);
		}
		return std::move(*result);
	}
};
template <>
class parse_response<validate_request> {
public:
	using response_type = bool;
	using result_type = boost::expected<bool, error>;
	template <typename Fields>
	static result_type parse (const beast::http::response<beast::http::string_body, Fields> & response) {
		switch (response.status) {
		case 204:
			return true;
		case 403:
			return false;
		default:
			break;
		}
		return parse_error(response);
	}
};
template <>
class parse_response<signout_request> : public void_parser {	};

template <
	typename AsyncStream,
	typename DynamicBuffer,
	typename Request,
	typename Allocator,
	typename Handler
>
class http_request_op {
private:
	using http_request_allocator_type = Allocator;
	using http_request_fields_type = beast::http::basic_fields<http_request_allocator_type>;
	using http_request_type = beast::http::request<
		beast::http::string_body,
		http_request_fields_type
	>;
	using http_response_allocator_type = beast::handler_alloc<char, Handler>;
	using http_response_fields_type = beast::http::basic_fields<http_response_allocator_type>;
	using http_response_parser_type = beast::http::message_parser<
		false,
		beast::http::string_body,
		http_response_fields_type
	>;
	using response_type = response_t<Request>;
	enum class stage {
		pending,
		write,
		read,
		done
	};
	class state {
	public:
		AsyncStream & stream;
		DynamicBuffer & buffer;
		//	The "typename" here is required by MSVC++ 2017
		//	but not by GCC 6.3.0 or Clang 4
		typename http_request_op::stage stage;
		http_request_type request;
		optional<http_response_parser_type> parser;
		state (
			Handler & h,
			AsyncStream & stream,
			DynamicBuffer & buffer,
			http_request_type req
		)	:	stream(stream),
				buffer(buffer),
				stage(http_request_op::stage::pending),
				request(std::move(req)),
				parser(
					in_place,
					std::piecewise_construct,
					std::forward_as_tuple(),
					std::forward_as_tuple(http_response_allocator_type(h))
				)
		{	}
	};
	using pointer = beast::handler_ptr<state, Handler>;
	pointer ptr_;
	void write () {
		beast::http::async_write(ptr_->stream, ptr_->request, std::move(*this));
	}
	void read () {
		assert(ptr_->parser);
		beast::http::async_read(ptr_->stream, ptr_->buffer, *ptr_->parser, std::move(*this));
	}
	template <typename T, typename Response = response_t<Request>>
	std::enable_if_t<std::is_same<Response, void>::value> invoke (T && e) {
		error err(std::forward<T>(e));
		ptr_.invoke(std::move(err));
	}
	template <typename T, typename Response = response_t<Request>>
	std::enable_if_t<!std::is_same<Response, void>::value> invoke (T && e) {
		error err(std::forward<T>(e));
		ptr_.invoke(std::move(err), Response{});
	}
	template <typename T>
	void invoke (boost::expected<T, error> ex) {
		if (ex) ptr_.invoke(error{}, std::move(*ex));
		else ptr_.invoke(std::move(ex.error()), T{});
	}
	void invoke (boost::expected<void, error> ex) {
		if (ex) ptr_.invoke(error{});
		else ptr_.invoke(std::move(ex.error()));
	}
	void done () {
		assert(ptr_->parser);
		invoke(parse_response<Request>::parse(ptr_->parser->get()));
	}
public:
	http_request_op () = delete;
	http_request_op (const http_request_op &) = default;
	http_request_op (http_request_op &&) = default;
	http_request_op & operator = (const http_request_op &) = default;
	http_request_op & operator = (http_request_op &&) = default;
	template <typename DeducedHandler>
	http_request_op (
		AsyncStream & stream,
		DynamicBuffer & buffer,
		http_request_type req,
		DeducedHandler && handler
	)	:	ptr_(std::forward<DeducedHandler>(handler), stream, buffer, std::move(req))
	{	}
	friend bool asio_handler_is_continuation (const http_request_op * op) noexcept {
		using boost::asio::asio_handler_is_continuation;
		switch (op->ptr_->stage) {
		case stage::pending:
		case stage::write:
			return asio_handler_is_continuation(std::addressof(op->ptr_.handler()));
		default:
			break;
		}
		return true;
	}
	friend void * asio_handler_allocate (std::size_t size, const http_request_op * op) {
		using boost::asio::asio_handler_allocate;
		return asio_handler_allocate(size, std::addressof(op->ptr_.handler()));
	}
	friend void asio_handler_deallocate (void * ptr, std::size_t size, const http_request_op * op) {
		using boost::asio::asio_handler_deallocate;
		return asio_handler_deallocate(ptr, size, std::addressof(op->ptr_.handler()));
	}
	template <typename F>
	friend void asio_handler_invoke (F && f, const http_request_op * op) {
		using boost::asio::asio_handler_invoke;
		return asio_handler_invoke(f, std::addressof(op->ptr_.handler()));
	}
	void operator () (beast::error_code ec, std::size_t num = 0) {
		(void)num;
		if (ec) {
			invoke(ec);
			return;
		}
		switch (ptr_->stage) {
		case stage::pending:
			ptr_->stage = stage::write;
			write();
			break;
		case stage::write:
			ptr_->stage = stage::read;
			read();
			break;
		case stage::read:
			ptr_->stage = stage::done;
			done();
			break;
		case stage::done:
			//	Should never be invoked in this state
		default:
			throw std::logic_error("Invalid invocation of mcpp::yggdrasil::detail::http_request_op::operator ()");
		}
	}
};

template <typename Request, typename Body, typename Fields>
void setup_request_method (const Request &, beast::http::request<Body, Fields> & request) {
	request.method("POST");
}

template <typename Request, typename Body, typename Fields>
void setup_request_content_type (const Request &, beast::http::request<Body, Fields> & request) {
	request.fields.replace("Content-Type", "application/json; charset=utf-8");
}

template <typename Body, typename Fields>
void setup_request_target (const authenticate_request &, beast::http::request<Body, Fields> & request) {
	request.target("/authenticate");
}
template <typename Body, typename Fields>
void setup_request_target (const refresh_request &, beast::http::request<Body, Fields> & request) {
	request.target("/refresh");
}
template <typename Body, typename Fields>
void setup_request_target (const validate_request &, beast::http::request<Body, Fields> & request) {
	request.target("/validate");
}
template <typename Body, typename Fields>
void setup_request_target (const signout_request &, beast::http::request<Body, Fields> & request) {
	request.target("/signout");
}

template <typename Request, typename Body, typename Fields>
void setup_request_body (const Request & req, beast::http::request<Body, Fields> & request) {
	request.body = to_json(req);
}

template <typename Request, typename Body, typename Fields>
void setup_request (const Request & req, beast::http::request<Body, Fields> & request) {
	request.version = 11;
	detail::setup_request_method(req, request);
	detail::setup_request_target(req, request);
	detail::setup_request_content_type(req, request);
	detail::setup_request_body(req, request);
};

template <typename Response>
class signature_helper {
public:
	using type = void (error, Response);
};
template <>
class signature_helper<void> {
public:
	using type = void (error);
};

template <typename Request>
using signature_t = typename signature_helper<response_t<Request>>::type;

}

/**
 *	Submits a Yggdrasil request and retrieves an
 *	appropriate response via a model of `AsyncStream`.
 *
 *	\tparam AsyncStream
 *		A model of `AsyncStream` which shall be used
 *		to send and receive HTTP requests.
 *	\tparam DynamicBuffer
 *		A model of `DynamicBuffer` which shall be used
 *		as scratch space for parsing.
 *	\tparam Request
 *		A Yggdrasil request.
 *	\tparam Allocator
 *		A model of `Allocator` which was used to allocate
 *		memory for the provided `beast::http::basic_fields`.
 *	\tparam CompletionToken
 *		The completion handler. Note that Boost.Asio fancy
 *		completion handlers (such as `boost::asio::use_future`)
 *		are valid here. The completion handler accepts two
 *		arguments: An \ref error as the first and an
 *		object appropriate to represent the response to
 *		a request of type \em Request as the second.
 *
 *	\param [in] stream
 *		The stream which shall be used to perform asynchronous
 *		read and write operations. This reference must remain
 *		valid for the duration of the asynchronous operation or
 *		the behavior is undefined.
 *	\param [in] buffer
 *		The buffer which shall be used for scratch space when
 *		parsing. This reference must remain valid for the duration
 *		of the asynchronous operation or the behavior is
 *		undefined.
 *	\param [in] request
 *		A Yggdrasil request object which shall be submitted. This
 *		reference need not remain valid after this function
 *		returns. It will already have been serialized to an HTTP
 *		request by then and the asynchronous operation will not
 *		require it further.
 *	\param [in] fields
 *		Any HTTP header fields the caller wishes to supplement
 *		(e.g. "User-Agent"). Note that the implementation reserves
 *		the right to overwrite certain header fields such as
 *		"Content-Type."
 *	\param [in] token
 *		The completion handler. The first argument shall be an
 *		\ref error. If this argument is truthy then the second
 *		argument must be ignored. The second argument shall be
 *		a response object appropriate for \em Request.
 *
 *	\return
 *		The value returned shall be appropriate given
 *		\em CompletionHandler. For example if \em CompletionHandler
 *		is `boost::asio::use_future` then the returned value shall
 *		be `std::future<Response>` where `Response` is the response
 *		object type appropriate given \em Request.
 */
template <
	typename AsyncStream,
	typename DynamicBuffer,
	typename Request,
	typename Allocator,
	typename CompletionToken
>
beast::async_return_type<
	CompletionToken,
	detail::signature_t<Request>
> async_http_request (
	AsyncStream & stream,
	DynamicBuffer & buffer,
	const Request & request,
	beast::http::basic_fields<Allocator> fields,
	CompletionToken && token
) {
	using fields_type = beast::http::basic_fields<Allocator>;
	using request_type = beast::http::request<beast::http::string_body, fields_type>;
	using header_type = beast::http::header<true, fields_type>;
	request_type req(header_type(std::move(fields)));
	detail::setup_request(request, req);
	beast::http::prepare(req);
	using Signature = detail::signature_t<Request>;
	beast::async_completion<CompletionToken, Signature> init(token);
	detail::http_request_op<
		AsyncStream,
		DynamicBuffer,
		Request,
		Allocator,
		beast::handler_type<CompletionToken, Signature>
	> op(
		stream,
		buffer,
		std::move(req),
		init.completion_handler
	);
	op(beast::error_code{}, 0);
	return init.result.get();
}

}
}
