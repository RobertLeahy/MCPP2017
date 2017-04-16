#include <mcpp/protocol/error.hpp>
#include <stdexcept>
#include <string>
#include <system_error>

namespace mcpp {
namespace protocol {

const std::string & to_string (error c) {
	static const std::string eof("Unexpected EOF");
	static const std::string unrep("Encoded value unrepresentable by destination type");
	static const std::string overl("Encoded representation longer than necessary");
	static const std::string overf("Integer overflow");
	static const std::string unexpected("Unexpected value");
	static const std::string inconsistent("Body shorter than indicated by length prefix");
	static const std::string uncompressed("Uncompressed data where compressed data was expected");
	static const std::string compressed("Compressed data where uncompressed data was expected");
	switch (c) {
	case error::end_of_file:
		return eof;
	case error::unrepresentable:
		return unrep;
	case error::overlong:
		return overl;
	case error::overflow:
		return overf;
	case error::unexpected:
		return unexpected;
	case error::inconsistent_length:
		return inconsistent;
	case error::uncompressed:
		return uncompressed;
	case error::compressed:
		return compressed;
	default:
		break;
	}
	throw std::logic_error("Unrecognized error code");
}

const std::error_category & error_category () {
	static const class final : public std::error_category {
	public:
		virtual const char * name () const noexcept override {
			return "Minecraft Protocol";
		}
		virtual std::string message (int condition) const override {
			error e = static_cast<error>(condition);
			return to_string(e);
		}
	} retr;
	return retr;
}

std::error_code make_error_code (error e) noexcept {
	return std::error_code(
		static_cast<int>(e),
		error_category()
	);
}

std::error_condition make_error_condition (error e) noexcept {
	return std::error_condition(
		static_cast<int>(e),
		error_category()
	);
}

}
}
