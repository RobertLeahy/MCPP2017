#include <boost/variant.hpp>
#include <mcpp/protocol/error.hpp>
#include <mcpp/protocol/error_code.hpp>
#include <string>
#include <utility>

namespace mcpp {
namespace protocol {

error::error (error_code c, std::size_t offset) : c_(c), msg_(to_string(c).c_str()), offset_(offset) {	}

error::error (error_code c, const char * str, std::size_t offset) noexcept : c_(c), msg_(str), offset_(offset) {	}

error::error (error_code c, std::string str, std::size_t offset) : c_(c), msg_(std::move(str)), offset_(offset) {	}

const char * error::what () const noexcept {
	class visitor {
	public:
		//	Provided to support Boost 1.55
		//
		//	Builds without this on Boost 1.61 but
		//	fails due to the fact visiting type
		//	doesn't have result_type on Boost 1.55
		using result_type = const char *;
		result_type operator () (const char * str) noexcept {
			return str;
		}
		result_type operator () (const std::string & str) noexcept {
			return str.c_str();
		}
	};
	visitor v;
	return boost::apply_visitor(v, msg_);
}

error_code error::code () const noexcept {
	return c_;
}

std::size_t error::offset () const noexcept {
	return offset_;
}

void error::offset (std::size_t offset) noexcept {
	offset_ = offset;
}

}
}
