#include <mcpp/protocol/error_code.hpp>
#include <stdexcept>
#include <string>

namespace mcpp {
namespace protocol {

const std::string & to_string (error_code c) {
	static const std::string eof("Unexpected EOF");
	static const std::string unrep("Encoded value unrepresentable by destination type");
	static const std::string overl("Encoded representation longer than necessary");
	switch (c) {
	case error_code::end_of_file:
		return eof;
	case error_code::unrepresentable:
		return unrep;
	case error_code::overlong:
		return overl;
	default:
		break;
	}
	throw std::logic_error("Unrecognized error code");
}

}
}
