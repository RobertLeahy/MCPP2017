#include <mcpp/protocol/state.hpp>
#include <stdexcept>
#include <string>

namespace mcpp {
namespace protocol {

const std::string & to_string (state s) {
	static const std::string h("handshaking");
	static const std::string p("play");
	static const std::string st("status");
	static const std::string l("login");
	switch (s) {
	case state::handshaking:
		return h;
	case state::play:
		return p;
	case state::status:
		return st;
	case state::login:
		return l;
	default:
		break;
	}
	throw std::logic_error("Unrecognized state");
}

}
}
