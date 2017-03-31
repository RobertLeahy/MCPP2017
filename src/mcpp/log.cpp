#include <mcpp/log.hpp>
#include <stdexcept>
#include <string>

namespace mcpp {

const std::string & log::to_string (level l) {
	static const std::string emergency("EMERGENCY");
	static const std::string alert("ALERT");
	static const std::string critical("CRITICAL");
	static const std::string error("ERROR");
	static const std::string warning("WARNING");
	static const std::string notice("NOTICE");
	static const std::string info("INFO");
	static const std::string debug("DEBUG");
	switch (l) {
	case level::emergency:
		return emergency;
	case level::alert:
		return alert;
	case level::critical:
		return critical;
	case level::error:
		return error;
	case level::warning:
		return warning;
	case level::notice:
		return notice;
	case level::info:
		return info;
	case level::debug:
		return debug;
	default:
		break;
	}
	throw std::logic_error("Unknown log level");
}

log::~log () noexcept {	}

void log::write (const std::string & component, std::string message, level l) {
	if (!ignored(l)) write_impl(component, message, l);
}

}
