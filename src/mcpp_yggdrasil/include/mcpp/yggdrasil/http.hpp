/**
 *	\file
 */

#pragma once

#include "authenticate.hpp"
#include "json.hpp"
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

template <typename Request>
using response_t = typename response_helper<Request>::type;

const boost::system::error_category & http_error_category ();

template <typename Request>
class parse_response {
public:
	using response_type = response_t<Request>;
	using result_type = boost::expected<response_type, beast::error_code>;
	template <typename Fields>
	static result_type parse (const beast::http::response<beast::http::string_body, Fields> & response) {
		if (response.status != 200) return boost::make_unexpected(
			beast::error_code(
				response.status,
				http_error_category()
			)
		);
		auto result = from_json<response_type>(response.body);
		if (!result) {
			auto && ec = result.error();
			assert(ec.category() == from_json_error_category());
			return boost::make_unexpected(
				beast::error_code(
					ec.value(),
					from_json_boost_error_category()
				)
			);
		}
		return std::move(*result);
	}
};

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
		http_request_op::stage stage;
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
	void invoke (beast::error_code ec, response_type response = response_type{}) {
		ptr_.invoke(ec, std::move(response));
	}
	void done () {
		assert(ptr_->parser);
		auto result = parse_response<Request>::parse(ptr_->parser->get());
		if (!result) invoke(result.error());
		else invoke(beast::error_code{}, std::move(*result));
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

class setup_request_method_post {
public:
	template <typename Request, typename Body, typename Fields>
	static void setup (const Request &, beast::http::request<Body, Fields> & request) {
		request.method("POST");
	}
};

template <typename Request>
class setup_request_method : public setup_request_method_post {	};

class setup_request_content_type_json {
public:
	template <typename Request, typename Body, typename Fields>
	static void setup (const Request &, beast::http::request<Body, Fields> & request) {
		request.fields.replace("Content-Type", "application/json; charset=utf-8");
	}
};

template <typename Request>
class setup_request_content_type : public setup_request_content_type_json {	};

template <typename Request>
class setup_request_target;
template <>
class setup_request_target<authenticate_request> {
public:
	template <typename Body, typename Fields>
	static void setup (const authenticate_request &, beast::http::request<Body, Fields> & request) {
		request.target("/authenticate");
	}
};

template <typename Request>
class setup_request_body {
public:
	template <typename Body, typename Fields>
	static void setup (const Request & req, beast::http::request<Body, Fields> & request) {
		request.body = to_json(req);
	}
};

template <typename Request>
class setup_request {
public:
	template <typename Body, typename Fields>
	static void setup (const Request & req, beast::http::request<Body, Fields> & request) {
		request.version = 11;
		setup_request_method<Request>::setup(req, request);
		setup_request_target<Request>::setup(req, request);
		setup_request_content_type<Request>::setup(req, request);
		setup_request_body<Request>::setup(req, request);
	}
};

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
 *		arguments: A `beast::error_code` as the first and an
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
 *		The completion handler. The first argument shall be a
 *		`beast::error_code`. If this argument is truthy then
 *		the second argument must be ignored. The second argument
 *		shall be a response object appropriate for \em Request.
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
	void (beast::error_code, detail::response_t<Request>)
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
	detail::setup_request<Request>::setup(request, req);
	beast::http::prepare(req);
	using Signature = void (beast::error_code, detail::response_t<Request>);
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
