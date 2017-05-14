#include <mcpp/protocol/direction.hpp>
#include <stdexcept>
#include <string>

namespace mcpp {
namespace protocol {

const std::string & to_string (direction d) {
	static const std::string c("clientbound");
	static const std::string s("serverbound");
	switch (d) {
	case direction::clientbound:
		return c;
	case direction::serverbound:
		return s;
	default:
		break;
	}
	throw std::logic_error("Unrecognized direction");
}

}
}
